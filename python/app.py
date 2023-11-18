from flask import Flask, render_template
from os.path import abspath
from board import Board

app = Flask(__name__, template_folder=abspath("../web/templates"), static_folder=abspath("../web/templates/static"))

@app.route("/")
@app.route("/index")
def index():
    return render_template("index.html")

if __name__ == "__main__":
    app.run(port=5000, host="0.0.0.0")