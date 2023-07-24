#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define MEMSZ 32768u
#define STKSZ  1024u
#define REGSZ     8u
#define PARSZ     3u

#define ISREG   (1u << 15)
#define NUMMASK (ISREG - 1u)
#define REGMASK (REGSZ - 1u)

static uint16_t mem[MEMSZ];
static uint16_t reg[REGSZ];
static uint16_t stk[STKSZ];
static uint16_t *par[PARSZ];
static uint16_t ip, sp, progsz;

typedef enum {
    HALT, SET, PUSH, POP, EQ, GT, JMP, JT, JF,
    ADD, MULT, MOD, AND, OR, NOT, RMEM, WMEM,
    CALL, RET, OUT, IN, NOOP
} Opcode;

static const uint16_t parcount[] = {
    [HALT] = 0, [SET]  = 2, [PUSH] = 1, [POP] = 1,
    [EQ]   = 3, [GT]   = 3, [JMP]  = 1, [JT]  = 2, [JF] = 2,
    [ADD]  = 3, [MULT] = 3, [MOD]  = 3,
    [AND]  = 3, [OR]   = 3, [NOT]  = 2,
    [RMEM] = 2, [WMEM] = 2, [CALL] = 1, [RET] = 0,
    [OUT]  = 1, [IN]   = 1, [NOOP] = 0,
};

static bool push(uint16_t val)
{
    if (sp != STKSZ) {
        stk[sp++] = val;
        return true;
    }
    return false;
}

static bool pop(uint16_t * const val)
{
    if (sp) {
        *val = stk[--sp];
        return true;
    }
    return false;
}

static char nextchar(void)
{
    static char buf[256] = {0}, *c = buf;
    while (!c || *c == '\0')
        c = fgets(buf, sizeof buf, stdin);
    return *c++;
}

int main(void)
{
    FILE *f = fopen("challenge.bin", "rb");
    if (!f)
        return 1;
    progsz = (uint16_t)fread(mem, sizeof *mem, MEMSZ, f);
    fclose(f);

    bool run = true;
    while (run) {
        if (ip >= MEMSZ) { run = false; break; }
        if (mem[ip] > NOOP) { run = false; break; }
        Opcode op = mem[ip++];
        if (ip + parcount[op] > MEMSZ) { run = false; break; }
        for (uint16_t i = 0; i < parcount[op]; ++i, ++ip)
            par[i] = mem[ip] & ISREG ? &reg[mem[ip] & REGMASK] : &mem[ip];
        switch (op) {
            case HALT: run = false; break;
            case SET : *par[0] = *par[1]; break;
            case PUSH: run = push(*par[0]); break;
            case POP : run = pop(par[0]); break;
            case EQ  : *par[0] = *par[1] == *par[2]; break;
            case GT  : *par[0] = *par[1] > *par[2]; break;
            case JMP : ip = *par[0]; break;
            case JT  : if (*par[0]) ip = *par[1]; break;
            case JF  : if (!*par[0]) ip = *par[1]; break;
            case ADD : *par[0] = (*par[1] + *par[2]) & NUMMASK; break;
            case MULT: *par[0] = (*par[1] * *par[2]) & NUMMASK; break;
            case MOD : *par[0] = *par[1] % *par[2]; break;
            case AND : *par[0] = *par[1] & *par[2]; break;
            case OR  : *par[0] = *par[1] | *par[2]; break;
            case NOT : *par[0] = (~*par[1]) & NUMMASK; break;
            case RMEM: *par[0] = mem[*par[1]]; break;
            case WMEM: mem[*par[0]] = *par[1]; break;
            case CALL: run = push(ip); ip = *par[0]; break;
            case RET : run = pop(&ip); break;
            case OUT : putchar(*par[0]); break;
            case IN  : *par[0] = (uint16_t)nextchar(); break;
            case NOOP: break;
        }
    }
    return 0;
}
