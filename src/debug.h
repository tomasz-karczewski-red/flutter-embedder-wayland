// Copyright (c) 2021 Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//.
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
#pragma once

namespace flutter {

void dbg_init();

void dbgE(const char *format, ...) __attribute__((format(printf, 1, 2)));
void dbgW(const char *format, ...) __attribute__((format(printf, 1, 2)));
void dbgI(const char *format, ...) __attribute__((format(printf, 1, 2)));
void dbgT(const char *format, ...) __attribute__((format(printf, 1, 2)));

} // namespace flutter
