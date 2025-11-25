#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
AirFryerVM - Maquina Virtual para AirFryerScript
=================================================

Esta VM estende o conceito da MicrowaveVM com suporte completo
para as construcoes da linguagem AirFryerScript.

Arquitetura:
-----------
- Registradores de escrita: TIME, POWER, R0, R1, R2, R3
- Sensores read-only: TEMP, WEIGHT, MODE, STATE
- Memoria: pilha (stack)
- String table: para literais de texto

Conjunto de Instrucoes (ISA):
----------------------------
Instrucoes basicas (compativeis com MicrowaveVM):
  SET R n           - Define registrador R = n
  INC R             - Incrementa R
  DEC R             - Decrementa R
  DECJZ R label     - Se R == 0 vai para label, senao R = R - 1
  GOTO label        - Pula para label
  PUSH R            - Empilha valor de R
  POP R             - Desempilha para R
  HALT              - Para a execucao

Instrucoes aritmeticas (inteiros):
  ADD R1 R2         - R1 = R1 + R2
  SUB R1 R2         - R1 = R1 - R2
  MUL R1 R2         - R1 = R1 * R2
  DIV R1 R2         - R1 = R1 / R2
  MOD R1 R2         - R1 = R1 % R2

Instrucoes aritmeticas (fixed-point para tipo frac):
  ADDF R1 R2        - R1 = R1 + R2 (frac)
  SUBF R1 R2        - R1 = R1 - R2 (frac)
  MULF R1 R2        - R1 = (R1 * R2) / 100
  DIVF R1 R2        - R1 = (R1 * 100) / R2
  ITOF R            - R = R * 100
  FTOI R            - R = R / 100

Instrucoes de comparacao (resultado em R0):
  EQ R1 R2          - R0 = (R1 == R2)
  NE R1 R2          - R0 = (R1 != R2)
  LT R1 R2          - R0 = (R1 < R2)
  LE R1 R2          - R0 = (R1 <= R2)
  GT R1 R2          - R0 = (R1 > R2)
  GE R1 R2          - R0 = (R1 >= R2)

Instrucoes logicas:
  AND R1 R2         - R1 = R1 && R2
  OR R1 R2          - R1 = R1 || R2
  NOT R             - R = !R

Instrucoes de salto condicional:
  JZ R label        - Se R == 0 vai para label
  JNZ R label       - Se R != 0 vai para label

Instrucoes de impressao:
  PRINT             - Imprime TIME (compatibilidade)
  PRINTI R          - Imprime R como inteiro
  PRINTF R          - Imprime R como frac (divide por 100)
  PRINTB R          - Imprime R como bool (verdadeiro/falso)
  SPRINT id         - Imprime string da string table

Instrucoes de string:
  SDEF id "texto"   - Define string na string table

Instrucoes tematicas (air fryer):
  SETMODE n         - Define modo (0=manual, 1=batata, 2=legumes, 3=nuggets, 4=esfihas)
  PAUSE             - Pausa execucao (STATE=2)
  RESUME            - Resume execucao (STATE=1)
  STOP              - Para execucao (STATE=0, POWER=0)
"""

from dataclasses import dataclass
from typing import List, Dict, Tuple, Optional

@dataclass
class Instr:
    """Representa uma instrucao da VM"""
    op: str
    args: Tuple[str, ...]
    line_num: int  # Para mensagens de erro

class AirFryerVM:
    """
    Maquina Virtual para AirFryerScript
    """

    def __init__(self):
        # Registradores de escrita
        self.registers: Dict[str, int] = {
            "TIME": 0,
            "POWER": 0,
            "R0": 0,
            "R1": 0,
            "R2": 0,
            "R3": 0
        }
        
        # Sensores read-only
        self.readonly_registers: Dict[str, int] = {
            "TEMP": 0,      # Temperatura atual
            "WEIGHT": 100,  # Peso em gramas
            "MODE": 0,      # Modo da air fryer
            "STATE": 0      # Estado (0=parado, 1=ativo, 2=pausado)
        }
        
        # String table
        self.strings: Dict[int, str] = {}
        
        # Pilha
        self.stack: List[int] = []
        
        # Programa e controle
        self.program: List[Instr] = []
        self.labels: Dict[str, int] = {}
        self.pc: int = 0
        self.halted: bool = False
        self.steps: int = 0
        self.max_steps: int = 100000  # Limite para evitar loops infinitos

    def load_program(self, source: str):
        """
        Carrega um programa assembly
        """
        self.program.clear()
        self.labels.clear()
        self.strings.clear()
        self.stack.clear()
        self.pc = 0
        self.halted = False
        self.steps = 0
        
        # Resetar registradores
        for reg in self.registers:
            self.registers[reg] = 0
        self.readonly_registers["TEMP"] = 0
        self.readonly_registers["WEIGHT"] = 100
        self.readonly_registers["MODE"] = 0
        self.readonly_registers["STATE"] = 0
        
        lines = source.splitlines()
        
        # Primeira passagem: coletar labels e processar SDEF
        idx = 0
        for line_num, raw_line in enumerate(lines, 1):
            # Remover comentarios
            line = raw_line.split(';', 1)[0].strip()
            if not line:
                continue
            
            # Labels
            if line.endswith(':'):
                label = line[:-1].strip()
                if label:
                    if label in self.labels:
                        raise ValueError(f"Linha {line_num}: Label duplicado: {label}")
                    self.labels[label] = idx
                continue
            
            # SDEF (definicao de string)
            if line.upper().startswith('SDEF'):
                tokens = line.split(None, 2)
                if len(tokens) < 3:
                    raise ValueError(f"Linha {line_num}: SDEF requer id e texto")
                try:
                    str_id = int(tokens[1])
                    # Extrair string entre aspas
                    text_part = tokens[2]
                    if text_part.startswith('"') and text_part.endswith('"'):
                        text = text_part[1:-1]
                    else:
                        raise ValueError("String deve estar entre aspas")
                    self.strings[str_id] = text
                except (ValueError, IndexError) as e:
                    raise ValueError(f"Linha {line_num}: Erro no SDEF: {e}")
                continue
            
            # Instrucao normal
            idx += 1
        
        # Segunda passagem: parsear instrucoes
        for line_num, raw_line in enumerate(lines, 1):
            line = raw_line.split(';', 1)[0].strip()
            if not line or line.endswith(':') or line.upper().startswith('SDEF'):
                continue
            
            tokens = line.replace(',', ' ').split()
            if not tokens:
                continue
            
            op = tokens[0].upper()
            args = tuple(tokens[1:])
            
            # Validacao basica
            self._validate_instruction(op, args, line_num)
            
            self.program.append(Instr(op, args, line_num))

    def _validate_instruction(self, op: str, args: Tuple[str, ...], line_num: int):
        """
        Valida uma instrucao (verificacao basica)
        """
        valid_regs = {"TIME", "POWER", "R0", "R1", "R2", "R3"}
        
        # Instrucoes sem argumentos
        if op in ["HALT", "PRINT", "PAUSE", "RESUME", "STOP"]:
            if len(args) != 0:
                raise ValueError(f"Linha {line_num}: {op} nao aceita argumentos")
        
        # Instrucoes com um registrador
        elif op in ["INC", "DEC", "PUSH", "POP", "NOT", "ITOF", "FTOI", "PRINTI", "PRINTF", "PRINTB"]:
            if len(args) != 1:
                raise ValueError(f"Linha {line_num}: {op} requer 1 argumento")
            if args[0].upper() not in valid_regs:
                raise ValueError(f"Linha {line_num}: Registrador invalido: {args[0]}")
        
        # SET requer registrador e valor
        elif op == "SET":
            if len(args) != 2:
                raise ValueError(f"Linha {line_num}: SET requer registrador e valor")
            if args[0].upper() not in valid_regs:
                raise ValueError(f"Linha {line_num}: Registrador invalido: {args[0]}")
            try:
                int(args[1])
            except ValueError:
                raise ValueError(f"Linha {line_num}: SET requer valor inteiro")
        
        # Instrucoes com dois registradores
        elif op in ["ADD", "SUB", "MUL", "DIV", "MOD", "ADDF", "SUBF", "MULF", "DIVF",
                    "EQ", "NE", "LT", "LE", "GT", "GE", "AND", "OR"]:
            if len(args) != 2:
                raise ValueError(f"Linha {line_num}: {op} requer 2 argumentos")
            if args[0].upper() not in valid_regs or args[1].upper() not in valid_regs:
                raise ValueError(f"Linha {line_num}: Argumentos devem ser registradores validos")
        
        # Instrucoes com registrador e label
        elif op in ["DECJZ", "JZ", "JNZ"]:
            if len(args) != 2:
                raise ValueError(f"Linha {line_num}: {op} requer registrador e label")
            if args[0].upper() not in valid_regs:
                raise ValueError(f"Linha {line_num}: Primeiro argumento deve ser registrador")
        
        # Instrucoes com label
        elif op == "GOTO":
            if len(args) != 1:
                raise ValueError(f"Linha {line_num}: GOTO requer label")
        
        # SETMODE requer valor
        elif op == "SETMODE":
            if len(args) != 1:
                raise ValueError(f"Linha {line_num}: SETMODE requer valor")
            try:
                int(args[0])
            except ValueError:
                raise ValueError(f"Linha {line_num}: SETMODE requer valor inteiro")
        
        # SPRINT requer id
        elif op == "SPRINT":
            if len(args) != 1:
                raise ValueError(f"Linha {line_num}: SPRINT requer id da string")
            try:
                int(args[0])
            except ValueError:
                raise ValueError(f"Linha {line_num}: SPRINT requer id inteiro")
        
        else:
            raise ValueError(f"Linha {line_num}: Instrucao desconhecida: {op}")

    def step(self):
        """
        Executa uma instrucao
        """
        if self.halted:
            return
        
        if not (0 <= self.pc < len(self.program)):
            self.halted = True
            return
        
        self.steps += 1
        if self.steps > self.max_steps:
            raise RuntimeError(f"Limite de steps excedido ({self.max_steps}). Possivel loop infinito.")
        
        instr = self.program[self.pc]
        
        try:
            self._execute_instruction(instr)
        except Exception as e:
            raise RuntimeError(f"Erro na linha {instr.line_num}: {e}")

    def _execute_instruction(self, instr: Instr):
        """
        Executa uma instrucao especifica
        """
        op = instr.op
        args = instr.args
        
        # Helper para obter nome canonico de registrador
        def reg(name: str) -> str:
            return name.upper()
        
        # Helper para obter valor inteiro de argumento
        def val(arg: str) -> int:
            try:
                return int(arg)
            except ValueError:
                return self.registers[reg(arg)]
        
        # Instrucoes basicas
        if op == "SET":
            self.registers[reg(args[0])] = val(args[1])
            self.pc += 1
        
        elif op == "INC":
            self.registers[reg(args[0])] += 1
            self.pc += 1
        
        elif op == "DEC":
            self.registers[reg(args[0])] -= 1
            self.pc += 1
        
        elif op == "DECJZ":
            r = reg(args[0])
            if self.registers[r] == 0:
                label = args[1]
                if label not in self.labels:
                    raise ValueError(f"Label nao encontrado: {label}")
                self.pc = self.labels[label]
            else:
                self.registers[r] -= 1
                self.pc += 1
        
        elif op == "GOTO":
            label = args[0]
            if label not in self.labels:
                raise ValueError(f"Label nao encontrado: {label}")
            self.pc = self.labels[label]
        
        elif op == "PUSH":
            self.stack.append(self.registers[reg(args[0])])
            self.pc += 1
        
        elif op == "POP":
            if not self.stack:
                raise RuntimeError("POP em pilha vazia")
            self.registers[reg(args[0])] = self.stack.pop()
            self.pc += 1
        
        elif op == "HALT":
            print("\n=== PROGRAMA FINALIZADO ===")
            self.halted = True
        
        # Instrucoes aritmeticas (inteiros)
        elif op == "ADD":
            self.registers[reg(args[0])] += self.registers[reg(args[1])]
            self.pc += 1
        
        elif op == "SUB":
            self.registers[reg(args[0])] -= self.registers[reg(args[1])]
            self.pc += 1
        
        elif op == "MUL":
            self.registers[reg(args[0])] *= self.registers[reg(args[1])]
            self.pc += 1
        
        elif op == "DIV":
            divisor = self.registers[reg(args[1])]
            if divisor == 0:
                raise RuntimeError("Divisao por zero")
            self.registers[reg(args[0])] //= divisor
            self.pc += 1
        
        elif op == "MOD":
            divisor = self.registers[reg(args[1])]
            if divisor == 0:
                raise RuntimeError("Divisao por zero")
            self.registers[reg(args[0])] %= divisor
            self.pc += 1
        
        # Instrucoes aritmeticas (fixed-point)
        elif op == "ADDF":
            self.registers[reg(args[0])] += self.registers[reg(args[1])]
            self.pc += 1
        
        elif op == "SUBF":
            self.registers[reg(args[0])] -= self.registers[reg(args[1])]
            self.pc += 1
        
        elif op == "MULF":
            # Multiplicacao fixed-point: (a * b) / 100
            a = self.registers[reg(args[0])]
            b = self.registers[reg(args[1])]
            self.registers[reg(args[0])] = (a * b) // 100
            self.pc += 1
        
        elif op == "DIVF":
            # Divisao fixed-point: (a * 100) / b
            a = self.registers[reg(args[0])]
            b = self.registers[reg(args[1])]
            if b == 0:
                raise RuntimeError("Divisao por zero")
            self.registers[reg(args[0])] = (a * 100) // b
            self.pc += 1
        
        elif op == "ITOF":
            # Converter int para fixed-point
            self.registers[reg(args[0])] *= 100
            self.pc += 1
        
        elif op == "FTOI":
            # Converter fixed-point para int
            self.registers[reg(args[0])] //= 100
            self.pc += 1
        
        # Instrucoes de comparacao (resultado no primeiro operando)
        elif op == "EQ":
            r1 = reg(args[0])
            self.registers[r1] = 1 if self.registers[r1] == self.registers[reg(args[1])] else 0
            self.pc += 1
        
        elif op == "NE":
            r1 = reg(args[0])
            self.registers[r1] = 1 if self.registers[r1] != self.registers[reg(args[1])] else 0
            self.pc += 1
        
        elif op == "LT":
            r1 = reg(args[0])
            self.registers[r1] = 1 if self.registers[r1] < self.registers[reg(args[1])] else 0
            self.pc += 1
        
        elif op == "LE":
            r1 = reg(args[0])
            self.registers[r1] = 1 if self.registers[r1] <= self.registers[reg(args[1])] else 0
            self.pc += 1
        
        elif op == "GT":
            r1 = reg(args[0])
            self.registers[r1] = 1 if self.registers[r1] > self.registers[reg(args[1])] else 0
            self.pc += 1
        
        elif op == "GE":
            r1 = reg(args[0])
            self.registers[r1] = 1 if self.registers[r1] >= self.registers[reg(args[1])] else 0
            self.pc += 1
        
        # Instrucoes logicas
        elif op == "AND":
            self.registers[reg(args[0])] = 1 if (self.registers[reg(args[0])] and self.registers[reg(args[1])]) else 0
            self.pc += 1
        
        elif op == "OR":
            self.registers[reg(args[0])] = 1 if (self.registers[reg(args[0])] or self.registers[reg(args[1])]) else 0
            self.pc += 1
        
        elif op == "NOT":
            self.registers[reg(args[0])] = 0 if self.registers[reg(args[0])] else 1
            self.pc += 1
        
        # Instrucoes de salto condicional
        elif op == "JZ":
            if self.registers[reg(args[0])] == 0:
                label = args[1]
                if label not in self.labels:
                    raise ValueError(f"Label nao encontrado: {label}")
                self.pc = self.labels[label]
            else:
                self.pc += 1
        
        elif op == "JNZ":
            if self.registers[reg(args[0])] != 0:
                label = args[1]
                if label not in self.labels:
                    raise ValueError(f"Label nao encontrado: {label}")
                self.pc = self.labels[label]
            else:
                self.pc += 1
        
        # Instrucoes de impressao
        elif op == "PRINT":
            # Compatibilidade: imprime TIME
            print(self.registers["TIME"])
            self.pc += 1
        
        elif op == "PRINTI":
            # Imprime como inteiro
            print(self.registers[reg(args[0])], end=' ')
            self.pc += 1
        
        elif op == "PRINTF":
            # Imprime como frac (fixed-point / 100)
            value = self.registers[reg(args[0])]
            print(f"{value / 100:.2f}", end=' ')
            self.pc += 1
        
        elif op == "PRINTB":
            # Imprime como bool
            value = self.registers[reg(args[0])]
            print("verdadeiro" if value else "falso", end=' ')
            self.pc += 1
        
        elif op == "SPRINT":
            # Imprime string da tabela
            str_id = val(args[0])
            if str_id not in self.strings:
                raise ValueError(f"String id {str_id} nao encontrado")
            print(self.strings[str_id], end=' ')
            self.pc += 1
        
        # Instrucoes tematicas
        elif op == "SETMODE":
            mode = val(args[0])
            self.readonly_registers["MODE"] = mode
            self.readonly_registers["STATE"] = 1  # Ativa
            self.pc += 1
        
        elif op == "PAUSE":
            self.readonly_registers["STATE"] = 2  # Pausado
            self.pc += 1
        
        elif op == "RESUME":
            self.readonly_registers["STATE"] = 1  # Ativo
            self.pc += 1
        
        elif op == "STOP":
            self.readonly_registers["STATE"] = 0  # Parado
            self.registers["POWER"] = 0
            self.pc += 1
        
        else:
            raise ValueError(f"Instrucao desconhecida: {op}")

    def run(self):
        """
        Executa o programa ate HALT ou erro
        """
        while not self.halted:
            self.step()

    def state(self) -> Dict:
        """
        Retorna o estado atual da VM
        """
        return {
            "registers": dict(self.registers),
            "readonly": dict(self.readonly_registers),
            "stack": list(self.stack),
            "pc": self.pc,
            "halted": self.halted,
            "steps": self.steps
        }


def main():
    """
    Funcao principal para executar programas
    """
    import sys
    
    if len(sys.argv) < 2:
        print("Uso: python3 airfryer_vm.py <arquivo.mwasm>")
        print("\nOpcoes:")
        print("  -v, --verbose    Modo verbose (mostra estado apos cada instrucao)")
        print("  -d, --debug      Modo debug (passo a passo)")
        sys.exit(1)
    
    filename = sys.argv[1]
    verbose = "-v" in sys.argv or "--verbose" in sys.argv
    debug = "-d" in sys.argv or "--debug" in sys.argv
    
    try:
        with open(filename, 'r') as f:
            program = f.read()
    except FileNotFoundError:
        print(f"Erro: Arquivo '{filename}' nao encontrado.")
        sys.exit(1)
    
    vm = AirFryerVM()
    
    try:
        print(f"Carregando programa: {filename}")
        vm.load_program(program)
        print(f"Programa carregado: {len(vm.program)} instrucoes, {len(vm.strings)} strings\n")
        
        if debug:
            print("=== MODO DEBUG ===")
            print("Comandos: [enter]=proximo, q=sair, r=registradores, s=stack\n")
        
        print("=== EXECUTANDO ===\n")
        
        if debug:
            while not vm.halted:
                print(f"PC={vm.pc}: {vm.program[vm.pc].op} {' '.join(vm.program[vm.pc].args)}")
                cmd = input("> ").strip().lower()
                if cmd == 'q':
                    break
                elif cmd == 'r':
                    print("Registradores:", vm.registers)
                    continue
                elif cmd == 's':
                    print("Stack:", vm.stack)
                    continue
                vm.step()
                if verbose:
                    print("  Regs:", vm.registers)
        else:
            vm.run()
        
        print("\n\n=== ESTADO FINAL ===")
        print(f"Steps executados: {vm.steps}")
        print(f"Registradores: {vm.registers}")
        print(f"Sensores: {vm.readonly_registers}")
        if vm.stack:
            print(f"Stack: {vm.stack}")
        
    except Exception as e:
        print(f"\nERRO: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
