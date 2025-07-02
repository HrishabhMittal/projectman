# Projectman: A C++ Project handler

The goal of this project is to hopefully convert directory like this
```
something.cpp a.cpp main.cpp (all files directly included in each other)
```

to something like this 

```
src include Makefile README.md LICENSE .gitignore
```

with a single command:
```bash
projectman main.cpp 
```
