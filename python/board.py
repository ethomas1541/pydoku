import Pydoku

class Board(object):
    """
    Attributes
    ----------

    boardbytes:     Python bytes object containing an 81-byte string of the numbers on the board.
                    We can use some math to extrapolate their actual graphical positions on the board.

    status:         Status code int
                    0: File loaded properly
                    1: File not found
                    2: Invalid file size (!=89 bytes)
                    3: File characters are valid but already violate Sudoku rules
                    4: File contains invalid character 

    complete:       Int, treated as bool; becomes 1 when all tiles have their nums set to something nonzero
                    i.e., solved_tiles == 81 in the backend code

    

    """
    def __init__(self, filename:str) -> None:
        self.boardbytes = None
        self.complete = 0
        self.status = Pydoku.initialize("boards/" + filename + ".sudoku", self)
        
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