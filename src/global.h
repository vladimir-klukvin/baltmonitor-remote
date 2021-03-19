/**
 * @file global.h
 * @brief Global variable declaration and global type definition
 *
 * @author Vladimir Klukvin <vladimir.klukvin@yandex.com>
 * @copyright Copyright (c) 2021 Balt-System Ltd. <info@bsystem.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
