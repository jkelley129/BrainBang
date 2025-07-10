#!/usr/bin/env python3
"""
BrainBang to Brainfuck Compiler
A compiler that translates BrainBang (a more readable Brainfuck) to standard Brainfuck.
"""

import re
import sys
from typing import List, Optional

class BrainBangCompiler:
    def __init__(self):
        self.output = []
        self.current_indent = 0
        self.loop_stack = []
    
    def compile(self, source_code: str) -> str:
        """Compile BrainBang source code to Brainfuck."""
        self.output = []
        self.current_indent = 0
        self.loop_stack = []
        
        lines = source_code.split('\n')
        processed_lines = self._preprocess_lines(lines)
        
        for line_num, (indent_level, content) in enumerate(processed_lines, 1):
            try:
                self._process_line(indent_level, content)
            except Exception as e:
                raise SyntaxError(f"Error on line {line_num}: {e}")
        
        # Close any remaining loops
        while self.loop_stack:
            self.output.append(']')
            self.loop_stack.pop()
        
        return ''.join(self.output)
    
    def _preprocess_lines(self, lines: List[str]) -> List[tuple]:
        """Preprocess lines to handle comments and indentation."""
        processed = []
        
        for line in lines:
            # Remove comments
            if '//' in line:
                line = line[:line.index('//')]
            
            # Skip empty lines
            if not line.strip():
                continue
            
            # Calculate indentation level
            indent_level = 0
            for char in line:
                if char == ' ':
                    indent_level += 1
                elif char == '\t':
                    indent_level += 4  # Treat tab as 4 spaces
                else:
                    break
            
            content = line.strip()
            
            # Ensure line ends with semicolon (except for loop declarations)
            if content and not content.endswith(':') and not content.endswith(';'):
                raise SyntaxError(f"Line must end with semicolon: {content}")
            
            processed.append((indent_level, content))
        
        return processed
    
    def _process_line(self, indent_level: int, content: str):
        """Process a single line of BrainBang code."""
        if not content:
            return
        
        # Handle loop end based on indentation
        while self.loop_stack and indent_level <= self.loop_stack[-1]:
            self.output.append(']')
            self.loop_stack.pop()
        
        # Remove trailing semicolon for processing
        if content.endswith(';'):
            content = content[:-1]
        
        # Parse the statement
        if content.startswith('loop:'):
            self._handle_loop(indent_level)
        elif content.startswith('ent '):
            self._handle_ent(content[4:])  # Remove 'ent '
        elif content == 'cellout':
            self._handle_cellout()
        elif content == '<' or content == '>':
            self._handle_shift(content)
        elif content.startswith('<<') or content.startswith('>>'):
            self._handle_multi_shift(content)
        elif content.startswith('inc'):
            self._handle_inc(content)
        elif content.startswith('dec'):
            self._handle_dec(content)
        elif content == 'clr':
            self._handle_clr()
        else:
            raise SyntaxError(f"Unknown statement: {content}")
    
    def _handle_loop(self, indent_level: int):
        """Handle loop declaration."""
        self.output.append('[')
        self.loop_stack.append(indent_level)
    
    def _handle_ent(self, value_str: str):
        """Handle ent statement for entering values."""
        value_str = value_str.strip()
        
        if value_str == 'input':
            # Handle input - first clear cell, then read
            self.output.append('[-]')  # Clear current cell
            self.output.append(',')
        elif value_str.startswith("'") and value_str.endswith("'") and len(value_str) == 3:
            # Handle character literal - clear cell first, then set value
            char = value_str[1]
            ascii_val = ord(char)
            self.output.append('[-]')  # Clear current cell
            self.output.append('+' * ascii_val)
        elif value_str.startswith('"') and value_str.endswith('"'):
            # Handle string literal - clear each cell before setting
            string_content = value_str[1:-1]  # Remove quotes
            for i, char in enumerate(string_content):
                if i > 0:  # Move to next cell for subsequent characters
                    self.output.append('>')
                ascii_val = ord(char)
                self.output.append('[-]')  # Clear current cell
                self.output.append('+' * ascii_val)
        elif value_str.isdigit():
            # Handle integer literal - clear cell first, then set value
            num = int(value_str)
            if num > 255:
                raise ValueError(f"Value {num} is too large for a single cell (max 255)")
            self.output.append('[-]')  # Clear current cell
            self.output.append('+' * num)
        else:
            raise SyntaxError(f"Invalid value for ent: {value_str}")
    
    def _handle_cellout(self):
        """Handle cellout statement for printing."""
        self.output.append('.')
    
    def _handle_shift(self, direction: str):
        """Handle shift operators < and >."""
        self.output.append(direction)
    
    def _handle_multi_shift(self, shift_str: str):
        """Handle multi-shift operators like <<4 or >>3."""
        if shift_str.startswith('<<'):
            direction = '<'
            num_str = shift_str[2:]
        elif shift_str.startswith('>>'):
            direction = '>'
            num_str = shift_str[2:]
        else:
            raise SyntaxError(f"Invalid shift operator: {shift_str}")
        
        if not num_str.isdigit():
            raise SyntaxError(f"Invalid number in shift operator: {shift_str}")
        
        num_shifts = int(num_str)
        if num_shifts <= 0:
            raise SyntaxError(f"Number of shifts must be positive: {shift_str}")
        
        # Generate the repeated shift operations
        self.output.append(direction * num_shifts)
    
    def _handle_inc(self, inc_str: str):
        """Handle inc statement for incrementing cell values."""
        if inc_str.strip() == 'inc':
            # Simple increment by 1
            self.output.append('+')
        elif inc_str.startswith('inc '):
            # Increment by specified amount
            amount_str = inc_str[4:].strip()
            if not amount_str.isdigit():
                raise SyntaxError(f"Invalid increment amount: {amount_str}")
            amount = int(amount_str)
            if amount <= 0:
                raise SyntaxError(f"Increment amount must be positive: {amount}")
            self.output.append('+' * amount)
        else:
            raise SyntaxError(f"Invalid inc statement: {inc_str}")
    
    def _handle_dec(self, dec_str: str):
        """Handle dec statement for decrementing cell values."""
        if dec_str.strip() == 'dec':
            # Simple decrement by 1
            self.output.append('-')
        elif dec_str.startswith('dec '):
            # Decrement by specified amount
            amount_str = dec_str[4:].strip()
            if not amount_str.isdigit():
                raise SyntaxError(f"Invalid decrement amount: {amount_str}")
            amount = int(amount_str)
            if amount <= 0:
                raise SyntaxError(f"Decrement amount must be positive: {amount}")
            self.output.append('-' * amount)
        else:
            raise SyntaxError(f"Invalid dec statement: {dec_str}")
    
    def _handle_clr(self):
        """Handle clr statement for clearing cell values."""
        self.output.append('[-]')

def main():
    """Main function for command-line usage."""
    if len(sys.argv) != 2:
        print("Usage: python brainbang_compiler.py <source_file>")
        sys.exit(1)
    
    source_file = sys.argv[1]
    
    try:
        with open(source_file, 'r') as f:
            source_code = f.read()
        
        compiler = BrainBangCompiler()
        brainfuck_code = compiler.compile(source_code)
        
        # Output to stdout or save to file
        output_file = source_file.replace('.bb', '.bf')
        if output_file == source_file:
            output_file += '.bf'
        
        with open(output_file, 'w') as f:
            f.write(brainfuck_code)
        
    except FileNotFoundError:
        print(f"Error: File '{source_file}' not found")
        sys.exit(1)
    except Exception as e:
        print(f"Compilation error: {e}")
        sys.exit(1)

# Main execution
if __name__ == "__main__":
    main()