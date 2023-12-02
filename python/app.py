from flask import Flask, render_template
from os.path import abspath
from board import Board
from sys import argv
import Pydoku

FILENAME = None
BOARD = None

app = Flask(__name__, template_folder=abspath("web/templates"), static_folder=abspath("web/templates/static"))

@app.route("/")
@app.route("/index")
def index():
    return render_template("index.html")

@app.route("/_update_board")
def update():
    bb = BOARD.boardbytes
    Pydoku.step(BOARD)
    return bb

@app.route("/_update_guesses")
def updateGuesses():
    return BOARD.guessbits

@app.route("/_initialize", methods = ['POST'])
def initialize():
    global BOARD
    BOARD = Board(FILENAME)
    return 200

if __name__ == "__main__":
    FILENAME = argv[1]
    if len(argv) != 2:
        print("Usage:\n\tpython3 app.py {filename}")
        exit()
    BOARD = Board(FILENAME)
    if BOARD.status:
        print(f"Board did not load properly; status code {BOARD.status}")
        exit()
    app.run(port=5000, host="0.0.0.0")