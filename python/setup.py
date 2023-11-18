from distutils.core import setup, Extension

def main():
    setup(
        name="Pydoku",
        version="1.0.2",
        description="Python interface to C-based Sudoku solving algorithm",
        author="Elijah Thomas",
        author_email="ethomas1541@gmail.com",
        ext_modules=[Extension("Pydoku", ["c/pydoku.c"])]
    )

if __name__ == "__main__":
    main()