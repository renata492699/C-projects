#include <stdlib.h>
#include <string.h>

#include "capture.h"

int main(int argc, char *argv[])
{
    if (argc < 5) {
        fprintf(stderr, "Wrong number of arguments.\n");
        return -1;
    }

    uint8_t from_mask[4][1];
    char *from_mask_str = argv[2];
    uint8_t from_mask_n;
    char *to_mask_str = argv[3];
    uint8_t to_mask_n;
    uint8_t to_mask[4][1];
    char *statistic = argv[4];

    if ((strcmp(statistic, "flowstats") != 0) && (strcmp(statistic, "longestflow") != 0)) {
        fprintf(stderr,"Invalid last argument.\n");
        return -1;
    }

    if (sscanf(from_mask_str, "%hhu.%hhu.%hhu.%hhu/%hhu",
            from_mask[0], from_mask[1], from_mask[2], from_mask[3], &from_mask_n) != 5) {
        perror("could not parse mask.\n");
        return -1;
    }
    if (sscanf(to_mask_str, "%hhu.%hhu.%hhu.%hhu/%hhu",
           to_mask[0], to_mask[1], to_mask[2], to_mask[3], &to_mask_n) != 5) {
        perror("could not parse mask.\n");
        return -1;
    }

    if (from_mask_n > 32 || to_mask_n > 32) {
        fprintf(stderr, "vice jak 32 bitu.\n");
        return -1;
    }

    struct capture_t capture[1];
    struct capture_t filtered_from[1];
    struct capture_t filtered[1];

    if (load_capture(capture, argv[1]) != 0) {
        fprintf(stderr, "Load capture failed.\n");
        return -1;
    }

    if (filter_from_mask(capture, filtered_from, *from_mask, from_mask_n) != 0) {
        destroy_capture(capture);
        fprintf(stderr, "Filter from adress+mask failed.\n");
        return -1;
    }

    if (filter_to_mask(filtered_from, filtered, *to_mask, to_mask_n) != 0) {
        destroy_capture(capture);
        destroy_capture(filtered_from);
        fprintf(stderr, "filter to adress+mask failed.\n");
        return -1;
    }

    int (*print_statistic) (const struct capture_t *const capture) = NULL;
    if (strcmp(statistic, "flowstats") == 0) {
        print_statistic = print_flow_stats;
    } else if (strcmp(statistic, "longestflow") == 0) {
        print_statistic = print_longest_flow;
    }
    int rv = print_statistic(filtered);
    destroy_capture(capture);
    destroy_capture(filtered_from);
    destroy_capture(filtered);

    return rv;
}
