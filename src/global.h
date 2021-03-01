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

/*
 * C99 defines boolean type to be _Bool, but this doesn't match the format of
 * the other standard integer types. bool_t has been defined to fill this gap.
 */
typedef _Bool bool_t;

/*
 * C99 defines character type to be char, but this doesn't match the format of
 * the other standard integer types. char_t has been defined to fill this gap.
 */
typedef char char_t;

/*
 * Program name variable is provided by the libc
 */
extern const char_t *__progname;

#define PROGRAM_NAME __progname

#endif /* GLOBAL_H_ */
