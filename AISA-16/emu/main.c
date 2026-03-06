#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
        uint8_t memory[65536];
        union
        {
                uint16_t words[16];
                uint8_t bytes[32];
        } as;

        int cycle;
        uint8_t ir[2];
        bool ze, ov, cy, sg, od;
} CPU;

void cycle(CPU *cpu)
{
        switch (cpu->cycle)
        {
        case 0:
                cpu->ir[0] = cpu->memory[cpu->as.words[0]++];
                break;
        case 1:
                cpu->ir[1] = cpu->memory[cpu->as.words[0]++];
                break;
        case 2:
        {
                cpu->cycle = 0;
                const uint8_t opcode = cpu->ir[0] & 0xF;
                const uint8_t rd = (cpu->ir[0] >> 0x4) & 0xF;
                const uint8_t rs1 = cpu->ir[1] & 0xF;
                const uint8_t rs2 = (cpu->ir[1] >> 0x4) & 0xF;
                uint16_t LHS = cpu->as.words[rs1], RHS = cpu->as.words[rs2];
                switch (opcode)
                {
                case 1:
                        cpu->cy ^= 1;
                        RHS = ~RHS;
                case 0:
                {
                        cpu->as.words[rd] =
                            LHS +
                            RHS + cpu->cy;
                        cpu->ze = !cpu->as.words[rd];
                        if (opcode == 1)
                                cpu->cy = LHS < RHS;
                        else
                                cpu->cy = cpu->as.words[rd] < LHS || cpu->as.words[rd] < RHS;
                        cpu->sg = cpu->as.words[rd] & 0x8000 > 0;
                        cpu->ov = (LHS & 0x8000 > 0) != cpu->sg;
                        cpu->od = cpu->as.words[rd] & 0x1;
                        break;
                }
                case 2:
                        cpu->as.words[rd] =
                            LHS ^
                            RHS;
                        cpu->ze = !cpu->as.words[rd];
                        cpu->sg = cpu->as.words[rd] & 0x8000 > 0;
                        cpu->od = cpu->as.words[rd] & 0x1;
                        break;
                case 3:
                        cpu->as.words[rd] =
                            LHS |
                            RHS;
                        cpu->ze = !cpu->as.words[rd];
                        cpu->sg = cpu->as.words[rd] & 0x8000 > 0;
                        cpu->od = cpu->as.words[rd] & 0x1;
                        break;
                case 4:
                        cpu->as.words[rd] =
                            LHS &
                            RHS;
                        cpu->ze = !cpu->as.words[rd];
                        cpu->sg = cpu->as.words[rd] & 0x8000 > 0;
                        cpu->od = cpu->as.words[rd] & 0x1;
                        break;
                case 5: // LLI - Load Low Immediate
                {
                        uint8_t imm = cpu->ir[1]; // i8 is in second instruction byte
                        cpu->as.words[rd] = (cpu->as.words[rd] & 0xFF00) | imm;
                        // Flags? Spec says all operations set flags
                        cpu->ze = !cpu->as.words[rd];
                        cpu->sg = cpu->as.words[rd] & 0x8000;
                        cpu->od = cpu->as.words[rd] & 1;
                        break;
                }

                case 6: // LHI - Load High Immediate
                {
                        uint8_t imm = cpu->ir[1];
                        cpu->as.words[rd] = (cpu->as.words[rd] & 0x00FF) | (imm << 8);
                        cpu->ze = !cpu->as.words[rd];
                        cpu->sg = cpu->as.words[rd] & 0x8000;
                        cpu->od = cpu->as.words[rd] & 1;
                        break;
                }

                case 7: // Bcc - Conditional Branch
                {
                        int8_t offset = (int8_t)cpu->ir[1]; // sign-extended i8
                        uint8_t condition = rd;             // rd field holds i4 condition mask
                        bool group = (condition >> 3) & 1;
                        uint8_t mask = condition & 0x7;

                        // Evaluate condition based on flag group
                        bool take_branch = false;
                        uint8_t flags = 0;

                        if (group == 0)
                        {
                                flags = (cpu->ze ? 1 : 0) |
                                        (cpu->ov ? 2 : 0) |
                                        (cpu->cy ? 4 : 0);
                        }
                        else
                        {
                                flags = (cpu->sg ? 1 : 0) |
                                        (cpu->od ? 2 : 0) |
                                        (1 << 2); // $alw is always set?
                        }

                        // XOR with mask? Need to check spec
                        take_branch = (flags & mask) != 0;

                        if (take_branch)
                        {
                                cpu->as.words[0] += offset; // $pr is r0
                        }
                        break;
                }

                case 8: // LWL - Load Word Low
                {
                        int8_t offset = (int8_t)(rs2 << 4) >> 4; // sign-extend i4
                        uint16_t address = (int16_t)cpu->as.words[rs1] + offset;
                        cpu->as.words[rd] = (cpu->as.words[rd] & 0xFF00) | cpu->memory[address];
                        break;
                }

                case 9: // SWL - Store Word Low
                {
                        int8_t offset = (int8_t)(rs2 << 4) >> 4;
                        uint16_t address = (int16_t)cpu->as.words[rd] + offset;
                        cpu->memory[address] = cpu->as.words[rs1] & 0xFF;
                        break;
                }

                case 10: // LWH - Load Word High
                {
                        int8_t offset = (int8_t)(rs2 << 4) >> 4;
                        uint16_t address = (int16_t)cpu->as.words[rs1] + offset;
                        cpu->as.words[rd] = (cpu->as.words[rd] & 0x00FF) | (cpu->memory[address] << 8);
                        break;
                }

                case 11: // SWH - Store Word High
                {
                        int8_t offset = (int8_t)(rs2 << 4) >> 4;
                        uint16_t address = (int16_t)cpu->as.words[rd] + offset;
                        cpu->memory[address] = (cpu->as.words[rs1] >> 8) & 0xFF;
                        break;
                }

                case 12: // LLL - Load Low from Low
                        cpu->as.words[rd] = (cpu->as.words[rd] & 0xFF00) | (cpu->as.words[rs1] & 0xFF);
                        break;

                case 13: // LLH - Load Low from High
                        cpu->as.words[rd] = (cpu->as.words[rd] & 0xFF00) | ((cpu->as.words[rs1] >> 8) & 0xFF);
                        break;

                case 14: // LHL - Load High from Low
                        cpu->as.words[rd] = (cpu->as.words[rd] & 0x00FF) | ((cpu->as.words[rs1] & 0xFF) << 8);
                        break;

                case 15: // LHH - Load High from High
                        cpu->as.words[rd] = (cpu->as.words[rd] & 0x00FF) | (cpu->as.words[rs1] & 0xFF00);
                        break;
                }
                return;
        }
        default:
                cpu->cycle = 0;
                return;
        }
        cpu->cycle++;
}

int main(void)
{
        CPU cpu = {0};
}
