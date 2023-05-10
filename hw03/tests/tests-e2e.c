#define main student_main
#include "../nft.c"
#undef main

#include "cut.h"

#define TEST_FILE "test.pcap"

TEST(flow_stats)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "0.0.0.0/0", "0.0.0.0/0", "flowstats", NULL });

    CHECK(retval == 0);
    CHECK_FILE(
            stdout,
            "172.16.11.12 -> 74.125.19.17 : 3\n"
            "74.125.19.17 -> 172.16.11.12 : 2\n"
            "216.34.181.45 -> 172.16.11.12 : 3\n"
            "172.16.11.12 -> 216.34.181.45 : 2\n");
}

TEST(flow_stats_with_mask)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "172.16.11.5/24", "0.0.0.0/0", "flowstats", NULL });

    CHECK(retval == 0);
    CHECK_FILE(
            stdout,
            "172.16.11.12 -> 74.125.19.17 : 3\n"
            "172.16.11.12 -> 216.34.181.45 : 2\n");
}

TEST(flow_stats_with_mask_02)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "172.16.11.5/24", "106.0.0.0/8", "flowstats", NULL });

    CHECK(retval == 0);
    CHECK_FILE(
            stdout, "");
}

TEST(flow_stats_with_mask_03)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "172.0.0.0/8", "216.0.0.0/8", "flowstats", NULL });

    CHECK(retval == 0);
    CHECK_FILE(
            stdout,
            "172.16.11.12 -> 216.34.181.45 : 2\n");
}

TEST(flow_stats_with_mask_04)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "172.0.0.0/8", "216.0.0.0/8", "flowstats", NULL });

    CHECK(retval == 0);
    CHECK_FILE(
            stdout,
            "172.16.11.12 -> 216.34.181.45 : 2\n");
}

TEST(flow_stats_with_mask_05)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "172.0.0.0/8", "74.0.0.0/8", "flowstats", NULL });

    CHECK(retval == 0);
    CHECK_FILE(
            stdout,
            "172.16.11.12 -> 74.125.19.17 : 3\n");
}

TEST(flow_stats_with_mask_06)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "172.0.0.0/8", "74.0.0.0/0", "flowstats", NULL });

    CHECK(retval == 0);
    CHECK_FILE(
            stdout,
            "172.16.11.12 -> 74.125.19.17 : 3\n"
            "172.16.11.12 -> 216.34.181.45 : 2\n");
}

TEST(flow_stats_with_mask_07)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "172.0.0.0/8", "206.0.0.0/34", "flowstats", NULL });

    CHECK(retval == -1);

}

TEST(longest_flow)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "0.0.0.0/0", "0.0.0.0/0", "longestflow", NULL });

    CHECK(retval == 0);
    CHECK_FILE(
            stdout,
            "216.34.181.45 -> 172.16.11.12 : 1278472580:873217 - 1278472581:8800\n");
}

TEST(longest_flow_01)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "172.0.0.0/8", "74.0.0.0/8", "longestflow", NULL });

    CHECK(retval == 0);
    CHECK_FILE(
            stdout,
            "172.16.11.12 -> 74.125.19.17 : 1278472579:466743 - 1278472579:488369\n");
}

TEST(longest_flow_07)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "172.0.0/8", "74.0.0.0/8", "longestflow", NULL });

    CHECK(retval == -1);
}

TEST(longest_flow_02)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "74.0.0.0/8", "172.16.0.0/16", "longestflow", NULL });

    CHECK(retval == 0);
    CHECK_FILE(
            stdout,
            "74.125.19.17 -> 172.16.11.12 : 1278472579:488320 - 1278472579:489327\n");
}

TEST(longest_flow_03)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "74.0.0.0/8", "172.26.0.0/16", "longestflow", NULL });

    CHECK(retval == -1);
}

TEST(flow_empty_04)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "74.0.0.0/16", "172.0.0.0/16", "flowstats", NULL });

    CHECK(retval == 0);
    CHECK_FILE(stdout, "");
}

TEST(longest_flow_04)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "74.0.0.0/16", "172.0.0.0/16", "longestflow", NULL });

    CHECK(retval == -1);
}
