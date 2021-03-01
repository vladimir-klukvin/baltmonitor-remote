/**
 * @file global.h
 * @author V.K.
 * @brief Global variable declaration and global type definition
 * @date 2021-02-26
 *
 * @copyright Copyright. All rights reserved.
 *
 */
#ifndef GLOBAL_H_
#define GLOBAL_H_

typedef char char_t;

/*
 * Program name variable is provided by the libc
 */
extern const char_t *__progname;

#define PROGRAM_NAME __progname

#endif /* GLOBAL_H_ */
