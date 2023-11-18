import Pydoku
from board import Board

print(Board("blank").status)
print(Board("clank").status)
print(Board("badfile").status)
print(Board("invalid").status)
print(Board("badchar").status)

print()

print(Board("easy"))