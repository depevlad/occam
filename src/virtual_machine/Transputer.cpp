#include "include/Transputer.h"

void Transputer::doInstr(const u8 instrCode) {
    const u8  code = instrCode & 0xF0;
    const u32 oper = O | (instrCode & 0xF);

    switch (code) {
        /* ===== Build ===== */
        case 0x0: // pfix
            O = oper << 4;
            advanceInstr();
            break;
        case 0x1: // nfix
            O = (~oper) << 4;
            advanceInstr();
            break;
        case 0x2: // opr
            O = 0;
            advanceInstr();
            doOp(oper);
            break;

        /* ===== Memory Local ===== */
        case 0x3: // ldl
            C = B;
            B = A;
            O = 0;
            A = readWord(W + oper);
            advanceInstr();
            break;
        case 0x4: // stl
            writeWord(W + oper, A);
            A = B;
            B = C;
            O = 0;
            advanceInstr();
            break;
        case 0x5: // ldlp
            C = B;
            B = A;
            A = W + oper * 4;
            advanceInstr();
            break;

        /* ===== Constants ===== */
        case 0x6: // ldc
            O = 0;
            C = B;
            B = A;
            A = oper;
            advanceInstr();
            break;
        case 0x7: // adc
            A += oper;
            // Set overflow flag.
            O = 0;
            advanceInstr();
            break;
        case 0x8: // eqc
            A = (A == oper);
            O = 0;
            advanceInstr();
            break;

        /* ===== Jumps ===== */
        case 0x9: // j
            O = 0;
            I += 1 + oper;
            break;
        case 0xA: // cj
            O = 0;
            if (A == 0) {
                I += 1 + oper;
            } else {
                A = B;
                B = C;
                advanceInstr();
            }
            break;

        /* ===== Memory Non Local ===== */
        case 0xB: // ldnl
            if ((A & 4) != 0) {
                // Throw error here.
            } else {
                A = readWord(A + oper * 4);
                O = 0;
                advanceInstr();
            }
            break;
        case 0xC: // stnl
            if ((A & 4) != 0) {
                // Throw error here.
            } else {
                writeWord(A + oper * 4, B);
                O = 0;
                A = C;
                advanceInstr();
            }
            break;
        case 0xD: // ldnlp
            if ((A & 4) != 0) {
                // Throw error here.
            } else {
                A = A + oper * 4;
                O = 0;
                advanceInstr();
            }
            break;

        /* ===== Other ===== */
        case 0xE: // call
            break;
        case 0xF: // ajw
            O = 0;
            W += 4 * oper;
            break;
    }
}

void Transputer::doOp(const u8 opCode) {
    switch (opCode) {
        case 0x0:
            /* rev - reverse */
            std::swap(A, B);
            break;
        case 0x1:
            /* eqz - equal to zero */
            A = (A == 0);
            break;
        case 0x2:
            /* gt - greater */
            A = (B > A);
            C = (B > A);
            break;
        case 0x3:
            /* and */
            A = (A & B);
            break;
        case 0x4:
            /* or */
            A = (A | B);
            break;
        case 0x5:
            /* xor */
            A = (A ^ B);
            break;
        /* @TODO: Overflow for arithmetic operationns. */
        case 0x6:
            /* add */
            A = A + B;
            break;
        case 0x7:
            /* sub */
            A = A - B;
            break;
        case 0x8:
            /* mul */
            A = A * B;
            break;
        case 0x9:
            /* div */
            A = A / B;
            break;
        case 0xA:
            /* mod */
            A = A % B;
            break;
        case 0xB:
            /* shl - shift left */
            A <<= B;
            break;
        case 0xC:
            /* shr - shift right */
            A >>= B;
            break;
    }
}