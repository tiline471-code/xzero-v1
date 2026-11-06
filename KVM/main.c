#include <stdio.h>
#include "kvm.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <archivo.kyb>\n", argv[0]);
        return 1;
    }

    if (!kvm_run_file(argv[1])) {
        return 1;
    }

    return 0;
}
