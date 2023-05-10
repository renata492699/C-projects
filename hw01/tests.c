
#define CUT_MAIN
#include "cut.h"

#define main student_main
#include "main.c"
#undef main

TEST(nanecisto_1)
{
    INPUT_STRING(
            "; na vstupu se nenachazi zadny prikaz, komentar tedy nevyvola zadny vypis\n"
            "=\n"
            "");

    int main_rv = student_main();

    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
            "# 0\n"
            "");
}

TEST(nanecisto_2)
{
    INPUT_STRING(
            "P\n"
            "10\n"
            "\n"
            "43\n"
            "");

    int main_rv = student_main();
    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
            "# 1043\n"
            "");
}

TEST(nanecisto_binary)
{
    INPUT_STRING(
            "P T 110\n"
            "=\n"
            "");

    int main_rv = student_main();

    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
            "# 0\n"
            "# 6\n"
            "# 6\n"
            "");
}

TEST(nanecisto_minus)
{
    INPUT_STRING(
            "P 100\n"
            "- 50\n"
            "=");

    int main_rv = student_main();

    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
            "# 100\n"
            "# 50\n"
            "# 50\n"
            "");
}

TEST(nanecisto_octal)
{
    INPUT_STRING(
            "P O 72\n"
            "=\n"
            "O\n"
            "");

    int main_rv = student_main();

    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
            "# 0\n"
            "# 58\n"
            "# 58\n"
            "# 72\n"
            "");
}

TEST(nanecisto_zadani_3)
{
    INPUT_STRING(
            "P 21\n"
            "; okamzite provedeni prikazu vyse - ukonceni predchoziho vstupu\n"
            "\n"
            "P 22\n"
            "=\n"
            "\n"
            "; jednotlive prikazy nemusi byt oddeleny novym radkem\n"
            "P 23 =\n"
            "\n"
            "N\n"
            "");

    int main_rv = student_main();

    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
            "# 21\n"
            "# 22\n"
            "# 22\n"
            "# 23\n"
            "# 23\n"
            "# 0\n"
            "");
}

TEST(nanecisto_zadani_4)
{
    INPUT_STRING(
            "P 200 ;\n"
            "/ 5 ;\n"
            "% 6 ;\n"
            "=\n"
            "");

    int main_rv = student_main();

    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
            "# 200\n"
            "# 40\n"
            "# 4\n"
            "# 4\n"
            "");
}

TEST(nanecisto_zadani_5)
{
    INPUT_STRING(
            "P 10 ;\n"
            "< 3 ;\n"
            "");

    int main_rv = student_main();

    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
            "# 10\n"
            "# 80\n"
            "");
}

TEST(nanecisto_zadani_6)
{
    INPUT_STRING(
            "P 21 + 43 * 5 =\n"
            "");

    int main_rv = student_main();

    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
            "# 21\n"
            "# 64\n"
            "# 320\n"
            "# 320\n"
            "");
}

TEST(nanecisto_zadani_7)
{
    INPUT_STRING(
            "P 15 ;\n"
            "* X 1a ;\n"
            "O ;\n"
            "");

    int main_rv = student_main();
    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
            "# 15\n"
            "# F\n"
            "# 390\n"
            "# 606\n"
            "");
}

TEST(nanecisto_zadani_8)
{
    INPUT_STRING(
            "P 3 ;    akumulator = 3, pamet = 0\n"
            "M   ;    akumulator = 3, pamet = 3\n"
            "R   ;    akumulator = 3, pamet = 0\n"
            "M   ;    akumulator = 3, pamet = 3\n"
            "+ m ;    akumulator = 3 + 3 = 6, pamet = 3\n"
            "=\n"
            "");

    int main_rv = student_main();

    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
            "# 3\n"
            "# 6\n"
            "# 6\n"
            "");
}

TEST(01) {
    INPUT_STRING(
            "P 22\n"
            "=\n"
            "; okamzite provedeni prikazu vyse - ukonceni predchoziho vstupu\n"
            "");
    int main_rv = student_main();

    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
                "# 22\n"
                "# 22\n"
                "");
}

TEST(01_chyba) {
    INPUT_STRING(
            "P 35 / 0 =\n"
            "O\n"
            "");
    int main_rv = student_main();
    ASSERT(main_rv == 1);
}

TEST(02_chyba) {
    INPUT_STRING(
            "P 34 + T 1a =\n"
            "P 15;"
            "");
    int main_rv = student_main();
    ASSERT(main_rv == 1);
}

TEST(03_chyba) {
    INPUT_STRING(
            "k 14;"
            "");
    int main_rv = student_main();
    ASSERT(main_rv == 1);
}

TEST(04_chyba) {
    INPUT_STRING(
            "P T 12;\n"
            ""
            );
    int main_rv = student_main();
    ASSERT(main_rv == 1);
}

TEST(05_chyba) {
    INPUT_STRING(
            "X ab;\n"
            ""
    );
    int main_rv = student_main();
    ASSERT(main_rv == 1);
}

TEST(06_chyba) {
    INPUT_STRING(
            "P 0 M P 3 / m;\n"
            ""
    );
    int main_rv = student_main();
    ASSERT(main_rv == 1);
}


TEST(07_chyba) {
    INPUT_STRING(
            "P 3 ** 5;\n"
            ""
    );
    int main_rv = student_main();
    ASSERT(main_rv == 1);
}

TEST(08_chyba) {
    INPUT_STRING(
            "P O 19;\n"
            ""
    );
    int main_rv = student_main();
    ASSERT(main_rv == 1);
}

TEST(09_chyba) {
    INPUT_STRING(
            "P^J9;\n"
            ""
    );
    int main_rv = student_main();
    ASSERT(main_rv == 1);
}


TEST(my_01) {
    INPUT_STRING(
            "O T X;\n"
            "");
    int main_rv = student_main();
    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
                "# 0\n"
                "# 0\n"
                "# 0\n");
}

TEST(my_02) {
    INPUT_STRING(
            "P T010N;"
            "");
    int main_rv = student_main();
    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
                "# 0\n"
                "# 2\n"
                "# 0\n");
}

TEST(my_03) {
    INPUT_STRING(
            "P 0 T;"
            "");
    int main_rv = student_main();
    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
                "# 0\n"
                "# 0\n"
                );
}
TEST(my_04) {
    INPUT_STRING(
            "P X A     B;"
            "");
    int main_rv = student_main();
    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
                "# 0\n"
                "# 171\n"
    );
}
TEST(my_05) {
    INPUT_STRING(
            "P X aB;"
            "");
    int main_rv = student_main();
    ASSERT(main_rv == 0);
    ASSERT_FILE(stdout,
                "# 0\n"
                "# 171\n"
    );

}
