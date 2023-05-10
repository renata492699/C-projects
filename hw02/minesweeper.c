#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minesweeper.h"

#define UNUSED(A) (void) (A)

/* ************************************************************** *
 *                         HELP FUNCTIONS                         *
 * ************************************************************** */

bool is_wrong_flag(uint16_t cell)
{
    return ((strchr("jklmnopqrs", cell) != NULL) || (cell == 'W'));
}

bool is_flag(uint16_t cell)
{
    return ((cell == 'F') || (is_wrong_flag(cell)));
}

bool is_X(uint16_t cell)
{
    return ((strchr("abcdefghi", cell) != NULL) || cell == 'X');
}

bool is_mine(uint16_t cell)
{
    return ((cell == 'M') || (cell == 'F'));
}

bool is_revealed(uint16_t cell)
{
    return isdigit(cell);
}

int get_number(uint16_t cell)
{
    if (is_mine(cell)) {
        return 0;
    }
    if (cell == 'X') {
        return 0;
    }
    if (is_X(cell)) {
        return cell - '`'; //aby 1 bylo 'a'
    }
    if (is_wrong_flag(cell)) {
        return (cell == 'W') ? 0 : (cell - 'i'); //aby 1 bylo 'j'
    }
    return (cell - '0');
}

/* ************************************************************** *
 *                         INPUT FUNCTIONS                        *
 * ************************************************************** */

bool set_cell(uint16_t *cell, char val)
{
    // pokud je cell odkryte policko se sousedici minou,
    // reprezentuju jako kladne cislo
    if (cell == NULL) {
        return false;
    }
    if (isdigit(val)) {
        int num = val - '0';
        if ((num < 0) || (num > 8)) {
            return false;
        }
        *cell = val;
        return true;
    }

    val = (char) toupper(val);
    if (!((val == 'M') || (val == '.') || (val == 'X') || (val == 'F') || (val == 'W'))) {
        return false;
    }
    // pokud je cell odkryte policko, nesousedici s minou,
    // reprezentuju jako znak 0
    if (val == '.') {
        *cell = '0';
    } else {
        *cell = val;
    }
    return true;
}

int load_board(size_t rows, size_t cols, uint16_t board[rows][cols])
{
    int ch = 0;
    bool set_c = true;
    int row = 0;
    while (row < (int) rows) {
        int col = 0;
        while (col < (int) cols) {
            ch = getchar();
            if (ch == EOF) {
                return -1;
            }
            if (isspace(ch)) {
                continue;
            }
            set_c = set_cell(&board[row][col], (char) ch);
            if (set_c) {
                col += 1;
            }
        }
        row += 1;
    }
    // The postprocess function should be called at the end of the load_board function
    return postprocess(rows, cols, board);
}

int add_numbers(int row, int col, size_t rows, size_t cols, uint16_t board[rows][cols])
{
    uint16_t cell = board[row][col];
    if ((cell == 'X') || (isdigit(cell)) || (cell == 'W')) {
        int count_mines = 0;
        for (int neighbour_r = row - 1; neighbour_r <= row + 1; neighbour_r++) {
            if ((neighbour_r < 0) || (neighbour_r >= (int) rows)) {
                continue;
            }
            for (int neighbour_c = col - 1; neighbour_c <= col + 1; neighbour_c++) {
                if ((neighbour_c < 0) || (neighbour_c >= (int) cols)) {
                    continue;
                }
                if (is_mine(board[neighbour_r][neighbour_c])) {
                    count_mines += 1;
                }
            }
        }
        if (isdigit(cell) && (cell != '0') && ((cell - '0') != count_mines)) {
            return -1;
        }
        if ((cell == 'X') && (count_mines != 0)) {
            board[row][col] = count_mines + '`'; //aby 1 bylo 'a'
        } else if (cell == 'X') {
            board[row][col] = 'X';
        } else if ((cell == 'W') && (count_mines != 0)) {
            board[row][col] = count_mines + 'i'; //aby 1 bylo 'j
        } else if (cell == 'W') {
            board[row][col] = 'W';
        } else {
            board[row][col] = count_mines + '0';
        }
    }
    return 0;
}

int postprocess(size_t rows, size_t cols, uint16_t board[rows][cols])
{
    if (board == NULL) {
        return -1;
    }
    if ((rows < MIN_SIZE) || (cols < MIN_SIZE) || (rows > MAX_SIZE) || (cols > MAX_SIZE)) {
        return -1;
    }
    if (is_mine(board[0][0]) || is_mine(board[0][cols - 1]) || is_mine(board[rows - 1][0]) || is_mine(board[rows - 1][cols - 1])) {
        return -1;
    }
    int all_mines = 0;
    for (int row = 0; row < (int) rows; row++) {
        for (int col = 0; col < (int) cols; col++) {
            if (is_mine(board[row][col])) {
                all_mines += 1;
                continue;
            }
            // pokud cell je X, tak dopocitam sousedici miny a
            // zapisu jako pismeno od a
            // neodkryte policko bez sousedicich min je jako X
            // stejne tak dopocitam okolni miny u wrong_flag, od pismena j
            int success = add_numbers(row, col, rows, cols, board);
            if (success == -1) {
                return -1;
            }
        }
    }
    return (all_mines == 0) ? -1 : all_mines;
}

/* ************************************************************** *
 *                        OUTPUT FUNCTIONS                        *
 * ************************************************************** */

void print_line(size_t cols)
{
    printf("   ");
    for (int i = 0; i < (int) cols; i++) {
        printf("+---");
    }
    printf("+\n");
}

void print_cell(uint16_t cell, char *result)
{
    uint16_t s_cell = show_cell(cell);
    if (isdigit(s_cell)) {
        snprintf(result, 4, " %d ", (s_cell - '0'));
    } else if (s_cell == 'X') {
        snprintf(result, 4, "%c%c%c", s_cell, s_cell, s_cell);
    } else if (is_flag(cell)) {
        snprintf(result, 4, "_%c_", s_cell);
    } else if (cell == 'O') {
        snprintf(result, 4, " M ");
    } else {
        snprintf(result, 4, "   ");
    }
}

int print_board(size_t rows, size_t cols, uint16_t board[rows][cols])
{
    printf("   ");
    for (int head = 0; head < (int) cols; head++) {
        printf("%3d", head);
        printf(" ");
    }
    printf("\n");
    for (int row = 0; row < (int) rows; row++) {
        print_line(cols);
        printf("%2d", row);
        printf(" |");
        for (int col = 0; col < (int) cols; col++) {
            char result[4] = { 0 };
            print_cell(board[row][col], result);
            printf("%s|", result);
        }
        printf("\n");
    }
    print_line(cols);
    return 0;
}

char show_cell(uint16_t cell)
{
    if (cell == '0') {
        return ' ';
    }
    if (isdigit(cell)) {
        return cell;
    }
    if (is_X(cell)) {
        return 'X';
    }
    if (is_flag(cell)) {
        return 'F';
    }
    if (cell == 'M') {
        return 'X';
    }
    return cell;
}

/* ************************************************************** *
 *                    GAME MECHANIC FUNCTIONS                     *
 * ************************************************************** */

int reveal_cell(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col)
{
    if (board == NULL) {
        return -1;
    }
    if ((row > rows) || (col > cols) || ((int) row < 0) || ((int) col < 0)) {
        return -1;
    }
    int rev_single = reveal_single(&board[row][col]);
    if ((rev_single == 0) && (board[row][col] == '0')) {
        reveal_floodfill(rows, cols, board, row, col);
        return 0;
    }
    return rev_single;
}

int reveal_single(uint16_t *cell)
{
    if (cell == NULL) {
        return -1;
    }
    if (is_flag(*cell) || (is_revealed(*cell))) {
        return -1;
    }
    if (is_mine(*cell)) {
        *cell = 'O';
        return 1;
    }
    int num = get_number(*cell);
    *cell = num + '0';
    return 0;
}

void reveal_floodfill(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col)
{
    if ((row > rows) || (col > cols) || ((int) row < 0) || ((int) col < 0)) {
        return;
    }
    int u_r = (row + 1);
    int u_c = (col + 1);
    for (int neighbour_r = (row - 1); neighbour_r <= u_r; neighbour_r++) {
        if ((neighbour_r < 0) || (neighbour_r >= (int) rows)) {
            continue;
        }
        for (int neighbour_c = (col - 1); neighbour_c <= u_c; neighbour_c++) {
            if ((neighbour_c < 0) || (neighbour_c >= (int) cols)) {
                continue;
            }
            if ((neighbour_c == (int) col) && (neighbour_r == (int) row)) {
                continue;
            }
            if (is_revealed(board[neighbour_r][neighbour_c])) {
                continue;
            }
            reveal_single(&board[row][col]);
            if (is_wrong_flag(board[row][col])) {
                int num = get_number(board[row][col]);
                board[row][col] = num + '0';
            }
            if (isdigit(board[row][col]) && (board[row][col]) != '0') {
                continue;
            }
            reveal_floodfill(rows, cols, board, neighbour_r, neighbour_c);
        }
    }
}

int count_flags_mines(size_t rows, size_t cols, uint16_t board[rows][cols])
{
    int count_mines = 0;
    int count_flags = 0;
    for (int row = 0; row < (int) rows; row++) {
        for (int col = 0; col < (int) cols; col++) {
            if (board[row][col] == 'F') {
                count_mines += 1;
                count_flags += 1;
                continue;
            }
            if (is_wrong_flag(board[row][col])) {
                count_flags += 1;
            } else if (board[row][col] == 'M') {
                count_mines += 1;
            }
        }
    }
    return count_mines - count_flags;
}

int flag_cell(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col)
{
    uint16_t cell = board[row][col];
    if (is_revealed(cell)) {
        return -1;
    }
    if (is_flag(cell)) {
        if (cell == 'F') {
            board[row][col] = 'M';
        } else {
            board[row][col] = (cell == 'W') ? 'X' : (get_number(cell) + '`'); //aby 1 bylo 'a'
        }
    } else {
        if (cell == 'M') {
            board[row][col] = 'F';
        } else {
            board[row][col] = (cell == 'X') ? 'W' : (get_number(cell) + 'i');
        }
    }
    return count_flags_mines(rows, cols, board);
}

bool is_solved(size_t rows, size_t cols, uint16_t board[rows][cols])
{
    for (int row = 0; row < (int) rows; row++) {
        for (int col = 0; col < (int) cols; col++) {
            if (is_wrong_flag(board[row][col]) || (is_X(board[row][col]))) {
                return false;
            }
        }
    }
    return true;
}

/* ************************************************************** *
 *                         BONUS FUNCTIONS                        *
 * ************************************************************** */

int generate_random_board(size_t rows, size_t cols, uint16_t board[rows][cols], size_t mines)
{
    if ((mines == 0) || (mines >= rows * cols - 4)) {
        return -1;
    }
    for (int row = 0; row < (int) rows; row++) {
        for (int col = 0; col < (int) cols; col++) {
            board[row][col] = 'X';
        }
    }
    while (mines != 0) {
        uint16_t random_row = (rand() % rows);
        uint16_t random_col = (rand() % cols);
        if (((random_col == 0) && (random_row == 0)) ||
                ((random_col == 0) && (random_row == (rows - 1))) ||
                ((random_col == (cols - 1)) && (random_row == 0)) ||
                ((random_col == (cols - 1)) && (random_row == (rows - 1)))) {
            continue;
        }
        if (board[random_row][random_col] == 'M') {
            continue;
        }
        board[random_row][random_col] = 'M';
        mines -= 1;
    }

    // The postprocess function should be called at the end of the generate random board function
    return postprocess(rows, cols, board);
}

int find_mines(size_t rows, size_t cols, uint16_t board[rows][cols])
{
    // TODO: Implement me
    UNUSED(rows);
    UNUSED(cols);
    UNUSED(board);
    return -1;
}
