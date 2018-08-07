# Automaton Virtual Machine

## Primitive Data Types

| Type         | Description                | Size in Bytes |
|--------------|----------------------------|--------------:|
| uint8        | 8-bit unsigned int         |             1 |
| uint16       | 16-bit unsigned int        |             2 |
| uint32       | 32-bit unsigned int        |             4 |
| uint64       | 64-bit unsigned int        |             8 |
| uint128      | 128-bit unsigned int       |            16 |
| uint256      | 256-bit unsigned int       |            32 |
| uint512      | 512-bit unsigned int       |            64 |
| uint1024     | 1024-bit unsigned int      |           128 |
| uint         | unsigned big int           |     arbitrary |
| int8         | 8-bit signed int           |             1 |
| int16        | 16-bit signed int          |             2 |
| int32        | 32-bit signed int          |             4 |
| int64        | 64-bit signed int          |             8 |
| int128       | 128-bit signed int         |            16 |
| int256       | 256-bit signed int         |            32 |
| int512       | 512-bit signed int         |            64 |
| int1024      | 1024-bit signed int        |           128 |
| int          | signed big int             |     arbitrary |
| fixed8x8     | 8/8 integer/fractional     |             2 |
| fixed16x16   | 16/16 integer/fractional   |             4 |
| fixed32x32   | 32/32 integer/fractional   |             8 |
| fixed64x64   | 64/64 integer/fractional   |            16 |
| fixed128x128 | 128/128 integer/fractional |            32 |
| fixed256x256 | 256/256 integer/fractional |            64 |
| fixed512x512 | 512/512 integer/fractional |           128 |

## Opcodes

Integers are always converted to int or uint types. All opcodes that take int or uint will work with their fixed size versions prior to instruction execution.

| OP_CODE   | Inputs       | Result  | Description                       |
|-----------|--------------|---------|-----------------------------------|
| ADD       | (uint, uint) | uint    | Unsigned integer addition         |
| SUB       | (uint, uint) | uint    | Unsigned integer subtraction      |
| MUL       | (uint, uint) | uint    | Unsigned integer multiplication   |
| DIV       | (uint, uint) | uint    | Unsigned integer division         |
| MOD       | (uint, uint) | uint    | Unsigned integer modulo           |
| SADD      | (int, int)   | int     | Signed integer addition           |
| SSUB      | (int, int)   | int     | Signed integer subtraction        |
| SMUL      | (int, int)   | int     | Signed integer multiplication     |
| SDIV      | (int, int)   | int     | Signed integer division           |
| AND |
| XOR |
| OR |
| LT |
| GT |
| SLT |
| SGT |
| ISZERO |
| NOT |

ADD, SUB, NOT, LT, GT, SLT, SGT, EQ, ISZERO, AND, OR, XOR, BYTE, CALLDATALOAD,
MLOAD, MSTORE, MSTORE8, PUSH*, DUP*, SWAP
{MUL, DIV, SDIV, MOD, SMOD, SIGNEXTEND
ADDMOD, MULMOD, JUMP

## Exceptions

| Type                    | Description                |
|-------------------------|----------------------------|
| ERR_DIVISION_BY_ZERO    | Integer division by 0      |
| ERR_OVERFLOW            | Integer overflow           |
