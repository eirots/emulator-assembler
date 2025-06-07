#ifndef arith_h_
#define arith_h_

/**
 * @brief Add RS1 to RS2, store result in RD
 * @details
 */
void ADD();

/**
 * @brief Add Imm to RS1, store result in RD
 * @details
 */
void ADDI();

/**
 * @brief Subtract RS2 from RS1, store result in RD
 * @details
 */
void SUB();

/**
 * @brief Subtract Imm* from RS1, store result in RD
 * @details
 */
void SUBI();

/**
 * @brief Multiply RS1 by RS2, store result in RD
 * @details
 */
void MUL();

/**
 * @brief Multiply RS1 by IMM, store the result in RD
 * @details
 */
void MULI();

/**
 * @brief Perform unsigned integer division RS1 / RS2. Store quotient in RD
 * @details Division by zero shall result in an emulator error
 */
void DIV();

/**
 * @brief Store result of signed division RS1 / RS2 in RD.
 * @detailsDivision by zero shall result in an emulator error
 */
void SDIV();

/**
 * @brief Divide RS1 by IMM (signed), store the result in RD.
 * @details Division by zero shall result in an emulator error
 */
void DIVI();

#endif