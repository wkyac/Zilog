# Zilog
A hobby zilogz80 emulator. Made as an educational project so I could learn about emulation and low-level architecture in general. A special thanks to https://github.com/kpmiller/emulator101 for being a great resource in this endeavor. 

# Installation
```
cd build
cmake ..
make
```
You can now run the program from the /bin directory.

# Usage
```
Z80 State Initialized
65536KB Available
Welcome. For help, enter "help".
>help
Available commands are: 
	help		 -- Displays this message.
	disassemble	 -- Starts a prompt for disassembling a file.
	load		 -- Loads a file into memory starting at the given position.
	printmem	 -- Displays an ncurses window of the current memory of the machine.
	clearmem	 -- Zeroes out memory.
	run		 -- Runs whatever is currently loaded into memory.
	reset		 -- Resets the program counter.
	exit		 -- Exits the program.
>
```

# Task List (for v1.0)
[ ] Finish implementing the main instruction set.
  [ ] rra
  [ ] rla
  [ ] srl
  [ ] daa
  [ ] cpl
  [ ] cff
  [ ] scf
  [ ] di
  [ ] ei
  [ ] Flags for various instructions (add, sub)
[ ] Validate insturction set for correctness.
[ ] Insert proper timings for instructions to more closely emulate the Z80
