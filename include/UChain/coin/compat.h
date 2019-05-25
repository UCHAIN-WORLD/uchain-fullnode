/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain.
 *
 * UChain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UC_COMPAT_H
#define UC_COMPAT_H

#ifdef _MSC_VER
/* There is no <endian.h> for MSVC but it is always little endian. */
#ifndef __LITTLE_ENDIAN__
#undef __BIG_ENDIAN__
#define __LITTLE_ENDIAN__
#endif
#endif

#ifdef _MSC_VER
#define BC_C_INLINE __inline
#else
#define BC_C_INLINE inline
#endif

#endif
