#ifndef move_h_
#define move_h_

/**
 * @brief Move contents of RS to RD
 * @details
 * @return
 */
bool MOV();

/**
 * @brief Move IMM value into RD
 * @details
 * @return
 */
bool MOVI();

/**
 * @brief Load address into RD
 * @details
 * @return
 */
bool LDA();

/**
 * @brief Store integer in RS at address
 * @details
 * @return
 */
bool STR();

/**
 * @brief Load integer at Address to RD
 * @details
 * @return
 */
bool LDR();

/**
 * @brief Store least significant byte in RS at address
 * @details
 * @return
 */
bool STB();

/**
 * @brief Load byte at Address to RD
 * @details
 * @return
 */
bool LDB();

#endif