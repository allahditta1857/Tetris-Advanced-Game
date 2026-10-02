/*
    Project: Tetris Advanced Game
    Author: Allah Ditta
    Institution: Namal University, Mianwali
    Copyright: © 2025 Allah Ditta
    All rights reserved.

    This project is a console-based Tetris game developed in C++
    for academic and personal portfolio use.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <cctype>

using namespace std;

const int BOARD_ROWS = 25;
const int BOARD_COLS = 30;
const int SHAPE_SIZE = 4;
const char BLOCK_CHAR = (char)219;
const string SAVE_FILE = "tetris_save.dat";
const string HIGHSCORE_FILE = "highscore.txt";

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void setColor(int color) {
    SetConsoleTextAttribute(hConsole, color);
}

class Renderer {
public:
    static void gotoRowCol(int r, int c) {
        COORD pos = { static_cast<SHORT>(c), static_cast<SHORT>(r) };
        SetConsoleCursorPosition(hConsole, pos);
    }

    static void drawBorder() {
        setColor(15);
        for (int j = 0; j <= BOARD_COLS + 1; j++) {
            gotoRowCol(0, j); cout << '-';
            gotoRowCol(BOARD_ROWS + 1, j); cout << '-';
        }
        for (int i = 0; i <= BOARD_ROWS + 1; i++) {
            gotoRowCol(i, 0); cout << '|';
            gotoRowCol(i, BOARD_COLS + 1); cout << '|';
        }
        setColor(15);
    }
};

class Sound {
public:
    static void land() {
        Beep(1200, 80);
    }

    static void clear() {
        Beep(900, 160);
    }
};

class HighScoreManager {
    int highScore;
public:
    HighScoreManager() {
        highScore = 0;
        ifstream in(HIGHSCORE_FILE);
        if (in.good()) {
            in >> highScore;
        }
    }

    void save(int s) {
        if (s > highScore) {
            highScore = s;
            ofstream out(HIGHSCORE_FILE);
            if (out) out << highScore;
        }
    }

    int get() const {
        return highScore;
    }
};

class Shape {
public:
    char matrix[SHAPE_SIZE][SHAPE_SIZE];
    int color;

    Shape() { clear(); }

    void clear() {
        for (int i = 0; i < SHAPE_SIZE; i++) {
            for (int j = 0; j < SHAPE_SIZE; j++) {
                matrix[i][j] = ' ';
            }
        }
        color = 15;
    }

    void copyFrom(char src[SHAPE_SIZE][SHAPE_SIZE]) {
        for (int i = 0; i < SHAPE_SIZE; i++) {
            for (int j = 0; j < SHAPE_SIZE; j++) {
                matrix[i][j] = src[i][j];
            }
        }
    }

    void rotate() {
        char tmp[SHAPE_SIZE][SHAPE_SIZE];
        for (int i = 0; i < SHAPE_SIZE; i++) {
            for (int j = 0; j < SHAPE_SIZE; j++) {
                tmp[j][SHAPE_SIZE - 1 - i] = matrix[i][j];
            }
        }
        copyFrom(tmp);
    }

    static Shape randomShape() {
        Shape s;
        int r = rand() % 7;

        switch (r) {
        case 0: {
            char t[SHAPE_SIZE][SHAPE_SIZE] = {
                {' ', BLOCK_CHAR, BLOCK_CHAR, ' '},
                {BLOCK_CHAR, BLOCK_CHAR, ' ', ' '},
                {' ', ' ', ' ', ' '},
                {' ', ' ', ' ', ' '}
            };
            s.copyFrom(t);
            s.color = 12;
        } break;

        case 1: {
            char t[SHAPE_SIZE][SHAPE_SIZE] = {
                {' ', BLOCK_CHAR, ' ', ' '},
                {BLOCK_CHAR, BLOCK_CHAR, BLOCK_CHAR, ' '},
                {' ', ' ', ' ', ' '},
                {' ', ' ', ' ', ' '}
            };
            s.copyFrom(t);
            s.color = 13;
        } break;

        case 2: {
            char t[SHAPE_SIZE][SHAPE_SIZE] = {
                {' ', BLOCK_CHAR, ' ', ' '},
                {' ', BLOCK_CHAR, ' ', ' '},
                {' ', BLOCK_CHAR, ' ', ' '},
                {' ', BLOCK_CHAR, ' ', ' '}
            };
            s.copyFrom(t);
            s.color = 11;
        } break;

        case 3: {
            char t[SHAPE_SIZE][SHAPE_SIZE] = {
                {BLOCK_CHAR, BLOCK_CHAR, ' ', ' '},
                {BLOCK_CHAR, BLOCK_CHAR, ' ', ' '},
                {' ', ' ', ' ', ' '},
                {' ', ' ', ' ', ' '}
            };
            s.copyFrom(t);
            s.color = 14;
        } break;

        case 4: {
            char t[SHAPE_SIZE][SHAPE_SIZE] = {
                {BLOCK_CHAR, BLOCK_CHAR, ' ', ' '},
                {' ', BLOCK_CHAR, BLOCK_CHAR, ' '},
                {' ', ' ', ' ', ' '},
                {' ', ' ', ' ', ' '}
            };
            s.copyFrom(t);
            s.color = 10;
        } break;

        case 5: {
            char t[SHAPE_SIZE][SHAPE_SIZE] = {
                {' ', BLOCK_CHAR, ' ', ' '},
                {' ', BLOCK_CHAR, ' ', ' '},
                {' ', BLOCK_CHAR, BLOCK_CHAR, ' '},
                {' ', ' ', ' ', ' '}
            };
            s.copyFrom(t);
            s.color = 9;
        } break;

        case 6: {
            char t[SHAPE_SIZE][SHAPE_SIZE] = {
                {BLOCK_CHAR, ' ', ' ', ' '},
                {BLOCK_CHAR, ' ', ' ', ' '},
                {BLOCK_CHAR, BLOCK_CHAR, ' ', ' '},
                {' ', ' ', ' ', ' '}
            };
            s.copyFrom(t);
            s.color = 6;
        } break;
        }

        return s;
    }
};

class Board {
public:
    char grid[BOARD_ROWS][BOARD_COLS];
    int colors[BOARD_ROWS][BOARD_COLS];
    int score;
    bool gameOver;
    bool lastLineCleared;

    Board() {
        initialize();
        gameOver = false;
        lastLineCleared = false;
    }

    void initialize() {
        score = 0;
        gameOver = false;
        lastLineCleared = false;
        for (int i = 0; i < BOARD_ROWS; i++) {
            for (int j = 0; j < BOARD_COLS; j++) {
                grid[i][j] = ' ';
                colors[i][j] = 15;
            }
        }
    }

    bool canMove(const Shape& s, int r, int c) const {
        for (int i = 0; i < SHAPE_SIZE; i++) {
            for (int j = 0; j < SHAPE_SIZE; j++) {
                if (s.matrix[i][j] != ' ') {
                    int nr = r + i;
                    int nc = c + j;

                    if (nc < 0 || nc >= BOARD_COLS || nr >= BOARD_ROWS) {
                        return false;
                    }

                    if (nr >= 0 && grid[nr][nc] != ' ') {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    void placeShape(const Shape& s, int r, int c) {
        for (int i = 0; i < SHAPE_SIZE; i++) {
            for (int j = 0; j < SHAPE_SIZE; j++) {
                if (s.matrix[i][j] != ' ') {
                    int nr = r + i;
                    int nc = c + j;
                    if (nr >= 0 && nr < BOARD_ROWS && nc >= 0 && nc < BOARD_COLS) {
                        grid[nr][nc] = s.matrix[i][j];
                        colors[nr][nc] = s.color;
                    }
                }
            }
        }

        clearLines();

        if (isTopFilled()) {
            gameOver = true;
        }
    }

    void clearLines() {
        lastLineCleared = false;
        int cleared = 0;

        for (int i = BOARD_ROWS - 1; i >= 0; --i) {
            bool full = true;
            for (int j = 0; j < BOARD_COLS; j++) {
                if (grid[i][j] == ' ') {
                    full = false;
                    break;
                }
            }

            if (full) {
                cleared++;
                lastLineCleared = true;

                for (int k = i; k > 0; --k) {
                    for (int j = 0; j < BOARD_COLS; j++) {
                        grid[k][j] = grid[k - 1][j];
                        colors[k][j] = colors[k - 1][j];
                    }
                }

                for (int j = 0; j < BOARD_COLS; j++) {
                    grid[0][j] = ' ';
                    colors[0][j] = 15;
                }
                ++i;
            }
        }

        score += cleared * 10;
    }

    bool isTopFilled() const {
        for (int j = 0; j < BOARD_COLS; j++) {
            if (grid[0][j] != ' ') {
                return true;
            }
        }
        return false;
    }

    void drawBoard(const HighScoreManager& hm) const {
        for (int i = 0; i < BOARD_ROWS; i++) {
            for (int j = 0; j < BOARD_COLS; j++) {
                Renderer::gotoRowCol(i + 1, j + 1);
                if (grid[i][j] != ' ') {
                    setColor(colors[i][j]);
                    cout << grid[i][j];
                    setColor(15);
                }
                else {
                    cout << ' ';
                }
            }
        }

        Renderer::gotoRowCol(1, BOARD_COLS + 4);
        cout << "High Score: " << hm.get();

        Renderer::gotoRowCol(3, BOARD_COLS + 4);
        cout << "Score: " << score;

        Renderer::gotoRowCol(5, BOARD_COLS + 4);
        cout << "Controls: A D W S H";
    }

    void drawFalling(const Shape& s, int r, int c) const {
        for (int i = 0; i < SHAPE_SIZE; i++) {
            for (int j = 0; j < SHAPE_SIZE; j++) {
                if (s.matrix[i][j] != ' ') {
                    int dr = r + i;
                    int dc = c + j;
                    if (dr >= 0 && dr < BOARD_ROWS && dc >= 0 && dc < BOARD_COLS) {
                        Renderer::gotoRowCol(dr + 1, dc + 1);
                        setColor(s.color);
                        cout << s.matrix[i][j];
                        setColor(15);
                    }
                }
            }
        }
    }

    void eraseFalling(const Shape& s, int r, int c) const {
        for (int i = 0; i < SHAPE_SIZE; i++) {
            for (int j = 0; j < SHAPE_SIZE; j++) {
                if (s.matrix[i][j] != ' ') {
                    int dr = r + i;
                    int dc = c + j;
                    if (dr >= 0 && dr < BOARD_ROWS && dc >= 0 && dc < BOARD_COLS) {
                        Renderer::gotoRowCol(dr + 1, dc + 1);
                        if (grid[dr][dc] != ' ') {
                            setColor(colors[dr][dc]);
                            cout << grid[dr][dc];
                            setColor(15);
                        }
                        else {
                            cout << ' ';
                        }
                    }
                }
            }
        }
    }

    bool saveToFile(const Shape& falling, const Shape& nextShape, const Shape& holdShape,
                    bool holdUsed, int row, int col, int speed, int themeCode) const {
        ofstream out(SAVE_FILE, ios::binary | ios::trunc);
        if (!out) return false;

        const char magic[4] = { 'T', 'E', 'T', '1' };
        out.write(magic, 4);

        for (int i = 0; i < BOARD_ROWS; i++) {
            out.write((char*)grid[i], BOARD_COLS);
        }

        for (int i = 0; i < BOARD_ROWS; i++) {
            out.write((char*)colors[i], BOARD_COLS * sizeof(int));
        }

        out.write((char*)&score, sizeof(score));
        out.write((char*)&gameOver, sizeof(gameOver));
        out.write((char*)&lastLineCleared, sizeof(lastLineCleared));
        out.write((char*)falling.matrix, sizeof(falling.matrix));
        out.write((char*)&falling.color, sizeof(falling.color));
        out.write((char*)nextShape.matrix, sizeof(nextShape.matrix));
        out.write((char*)&nextShape.color, sizeof(nextShape.color));
        out.write((char*)holdShape.matrix, sizeof(holdShape.matrix));
        out.write((char*)&holdShape.color, sizeof(holdShape.color));
        out.write((char*)&holdUsed, sizeof(holdUsed));
        out.write((char*)&row, sizeof(row));
        out.write((char*)&col, sizeof(col));
        out.write((char*)&speed, sizeof(speed));
        out.write((char*)&themeCode, sizeof(themeCode));
        out.close();

        return true;
    }

    bool loadFromFile(Shape& falling, Shape& nextShape, Shape& holdShape,
                      bool& holdUsed, int& row, int& col, int& speed, int& themeCode) {
        ifstream in(SAVE_FILE, ios::binary);
        if (!in) return false;

        char magic[4];
        in.read(magic, 4);

        if (magic[0] != 'T' || magic[1] != 'E' || magic[2] != 'T' || magic[3] != '1') {
            in.close();
            return false;
        }

        for (int i = 0; i < BOARD_ROWS; i++) {
            in.read((char*)grid[i], BOARD_COLS);
        }

        for (int i = 0; i < BOARD_ROWS; i++) {
            in.read((char*)colors[i], BOARD_COLS * sizeof(int));
        }

        in.read((char*)&score, sizeof(score));
        in.read((char*)&gameOver, sizeof(gameOver));
        in.read((char*)&lastLineCleared, sizeof(lastLineCleared));
        in.read((char*)falling.matrix, sizeof(falling.matrix));
        in.read((char*)&falling.color, sizeof(falling.color));
        in.read((char*)nextShape.matrix, sizeof(nextShape.matrix));
        in.read((char*)&nextShape.color, sizeof(nextShape.color));
        in.read((char*)holdShape.matrix, sizeof(holdShape.matrix));
        in.read((char*)&holdShape.color, sizeof(holdShape.color));
        in.read((char*)&holdUsed, sizeof(holdUsed));
        in.read((char*)&row, sizeof(row));
        in.read((char*)&col, sizeof(col));
        in.read((char*)&speed, sizeof(speed));
        in.read((char*)&themeCode, sizeof(themeCode));
        in.close();

        return true;
    }
};

bool strictNumericChoice(int& out, int minOpt, int maxOpt) {
    string s;
    if (!(cin >> s)) return false;

    for (char c : s) {
        if (!isdigit((unsigned char)c)) {
            cout << "Invalid input! Enter digits only.\n";
            return false;
        }
    }

    try {
        out = stoi(s);
    }
    catch (...) {
        cout << "Invalid input!\n";
        return false;
    }

    if (out < minOpt || out > maxOpt) {
        cout << "Invalid choice. Enter a number from " << minOpt << " to " << maxOpt << ".\n";
        return false;
    }

    return true;
}

enum Theme {
    THEME_DEFAULT = 15,
    THEME_CYAN = 11,
    THEME_MAGENTA = 13,
    THEME_YELLOW = 14
};

class Game {
    Board board;
    Shape falling, nextShape, holdShape;
    bool holdUsed;
    int row, col;
    int speed;
    int themeCode;
    HighScoreManager hm;

    void drawHUDAndPreviews() {
        Renderer::gotoRowCol(7, BOARD_COLS + 4);
        cout << "Next:";
        for (int i = 0; i < SHAPE_SIZE; i++) {
            for (int j = 0; j < SHAPE_SIZE; j++) {
                Renderer::gotoRowCol(9 + i, BOARD_COLS + 4 + j);
                if (nextShape.matrix[i][j] != ' ') {
                    setColor(nextShape.color);
                    cout << nextShape.matrix[i][j];
                    setColor(15);
                }
                else {
                    cout << ' ';
                }
            }
        }

        Renderer::gotoRowCol(14, BOARD_COLS + 4);
        cout << "Hold:";
        for (int i = 0; i < SHAPE_SIZE; i++) {
            for (int j = 0; j < SHAPE_SIZE; j++) {
                Renderer::gotoRowCol(16 + i, BOARD_COLS + 4 + j);
                if (holdShape.matrix[i][j] != ' ') {
                    setColor(holdShape.color);
                    cout << holdShape.matrix[i][j];
                    setColor(15);
                }
                else {
                    cout << ' ';
                }
            }
        }
    }

    void spawnNewFromNext() {
        falling = nextShape;
        nextShape = Shape::randomShape();
        row = 0;
        col = (BOARD_COLS - SHAPE_SIZE) / 2;
        holdUsed = false;
    }

    void doHold() {
        if (holdUsed) return;

        if (holdShape.matrix[0][0] == ' ') {
            holdShape = falling;
            falling = nextShape;
            nextShape = Shape::randomShape();
        }
        else {
            swap(holdShape, falling);
        }

        row = 0;
        col = (BOARD_COLS - SHAPE_SIZE) / 2;
        holdUsed = true;
    }

    void autoSave() {
        board.saveToFile(falling, nextShape, holdShape, holdUsed, row, col, speed, themeCode);
    }

public:
    Game() {
        srand((unsigned)time(NULL));
        holdUsed = false;
        themeCode = THEME_DEFAULT;
        speed = 500;
    }

    void start() {
        while (true) {
            Renderer::drawBorder();
            int choice = 0;

            while (true) {
                Renderer::gotoRowCol(BOARD_ROWS + 3, 0);
                cout << "\n1. Continue Previous Game\n2. New Game\nChoose option: ";
                if (strictNumericChoice(choice, 1, 2)) break;
            }

            if (choice == 1) {
                bool holdUsedLocal = false;
                int savedRow = 0, savedCol = 0, savedSpeed = 0, savedTheme = THEME_DEFAULT;
                Shape savedFalling, savedNext, savedHold;
                savedFalling.clear();
                savedNext.clear();
                savedHold.clear();

                if (!board.loadFromFile(savedFalling, savedNext, savedHold, holdUsedLocal, savedRow, savedCol, savedSpeed, savedTheme)) {
                    cout << "No previous game found. Returning to menu.\n";
                    continue;
                }

                if (board.gameOver) {
                    cout << "Previous saved game was already over. Displaying final board.\n";
                    Renderer::drawBorder();
                    board.drawBoard(hm);
                    Renderer::gotoRowCol(BOARD_ROWS + 4, 0);
                    cout << "\n(Press Enter to return to menu)";
                    cin.ignore();
                    cin.get();
                    continue;
                }

                falling = savedFalling;
                nextShape = savedNext;
                holdShape = savedHold;
                holdUsed = holdUsedLocal;
                row = savedRow;
                col = savedCol;
                speed = savedSpeed > 0 ? savedSpeed : 500;
                themeCode = savedTheme;
                break;
            }
            else {
                int lvl = 0;

                while (true) {
                    Renderer::gotoRowCol(BOARD_ROWS + 3, 0);
                    cout << "\nSelect difficulty:\n1. Easy\n2. Normal\n3. Hard\nChoose: ";
                    if (strictNumericChoice(lvl, 1, 3)) break;
                }

                if (lvl == 1) speed = 800;
                else if (lvl == 2) speed = 500;
                else speed = 250;

                int t = 0;

                while (true) {
                    Renderer::gotoRowCol(BOARD_ROWS + 6, 0);
                    cout << "\nSelect theme:\n1. Default\n2. Cyan\n3. Magenta\n4. Yellow\nChoose: ";
                    if (strictNumericChoice(t, 1, 4)) break;
                }

                if (t == 1) themeCode = THEME_DEFAULT;
                else if (t == 2) themeCode = THEME_CYAN;
                else if (t == 3) themeCode = THEME_MAGENTA;
                else themeCode = THEME_YELLOW;

                board.initialize();
                falling = Shape::randomShape();
                nextShape = Shape::randomShape();
                holdShape.clear();
                holdUsed = false;
                row = 0;
                col = (BOARD_COLS - SHAPE_SIZE) / 2;
                break;
            }
        }

        system("cls");
        gameLoop();
    }

    void gameLoop() {
        DWORD lastFall = GetTickCount();
        HighScoreManager localHM = hm;
        Renderer::drawBorder();

        while (true) {
            board.drawBoard(localHM);
            drawHUDAndPreviews();
            board.drawFalling(falling, row, col);

            if (_kbhit()) {
                char ch = _getch();

                if ((ch == 'a' || ch == 'A') && board.canMove(falling, row, col - 1)) {
                    board.eraseFalling(falling, row, col);
                    col--;
                }
                else if ((ch == 'd' || ch == 'D') && board.canMove(falling, row, col + 1)) {
                    board.eraseFalling(falling, row, col);
                    col++;
                }
                else if ((ch == 's' || ch == 'S')) {
                    if (board.canMove(falling, row + 1, col)) {
                        board.eraseFalling(falling, row, col);
                        row++;
                    }
                }
                else if ((ch == 'w' || ch == 'W')) {
                    Shape backup = falling;
                    falling.rotate();
                    if (!board.canMove(falling, row, col)) {
                        falling = backup;
                    }
                }
                else if ((ch == 'h' || ch == 'H')) {
                    board.eraseFalling(falling, row, col);
                    doHold();
                }
            }

            DWORD now = GetTickCount();
            if (now - lastFall >= (DWORD)speed) {
                if (board.canMove(falling, row + 1, col)) {
                    board.eraseFalling(falling, row, col);
                    row++;
                }
                else {
                    board.placeShape(falling, row, col);
                    Sound::land();

                    if (board.lastLineCleared) {
                        Sound::clear();
                    }

                    autoSave();
                    localHM.save(board.score);

                    if (board.gameOver) {
                        board.drawBoard(localHM);
                        Renderer::gotoRowCol(BOARD_ROWS + 3, 0);
                        setColor(12);
                        cout << "\nGAME OVER!\n";
                        setColor(15);

                        localHM.save(board.score);
                        remove(SAVE_FILE.c_str());

                        Renderer::gotoRowCol(BOARD_ROWS + 5, 0);
                        cout << "Press Enter to exit.";
                        cin.ignore();
                        cin.get();
                        exit(0);
                    }

                    spawnNewFromNext();
                }
                lastFall = now;
            }

            Sleep(30);
            board.eraseFalling(falling, row, col);
        }
    }
};

int main() {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &info);

    Game g;
    g.start();

    return 0;
}
