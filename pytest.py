import Pydoku

class Board(object):
    def __init__(self) -> None:
        self.boardstr = ""

print(Pydoku.initialize("boards/blank.sudoku"))
print(Pydoku.initialize("boards/clank.sudoku"))
print(Pydoku.initialize("boards/badfile.sudoku"))
print(Pydoku.initialize("boards/invalid.sudoku"))
print(Pydoku.initialize("boards/badchar.sudoku"))

Pydoku.initialize("boards/easy.sudoku")
Pydoku.print()