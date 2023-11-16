import Pydoku

class Board(object):
    def __init__(self, filename:str) -> None:
        self.boardbytes = None
        self.complete = 0
        self.init_status = Pydoku.initialize("boards/" + filename + ".sudoku", self)

    def __str__(self):
        ret = ""

        if self.boardbytes:
            boardstr = self.boardbytes.decode("utf-8")

            for i in range(81):
                if not ((i+1)%9):
                    ret += boardstr[i] + "\n"
                elif not ((i+1)%3):
                    ret += boardstr[i] + " | "
                else:
                    ret += boardstr[i] + " "
                if not ((i+1)%27) and i < 80:
                    ret += "------+-------+------\n"

            return ret
        else:
            raise IndexError

print(Board("blank").init_status)
print(Board("clank").init_status)
print(Board("badfile").init_status)
print(Board("invalid").init_status)
print(Board("badchar").init_status)

print()

print(Board("easy"))