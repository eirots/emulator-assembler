#ifndef move_h_
#define move_h_

/**
 * @brief Move contents of RS to RD
 * @details
 * @return
 */
void MOV();

/**
 * @brief Move IMM value into RD
 * @details
 * @return
 */
void MOVI();

/**
 * @brief Load address into RD
 * @details
 * @return
 */
void LDA();

/**
 * @brief Store integer in RS at address
 * @details
 * @return
 */
void STR();

/**
 * @brief Load integer at Address to RD
 * @details
 * @return
 */
void LDR();

/**
 * @brief Store least significant byte in RS at address
 * @details
 * @return
 */
void STB();

/**
 * @brief Load byte at Address to RD
 * @details
 * @return
 */
void LDB();

#endif