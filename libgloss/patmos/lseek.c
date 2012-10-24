// Copyright 2012 Florian Brandner
// Copyright 2012 Stefan Hepp
// 
// This file is part of the newlib C library for the Patmos processor.
// 
// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// (COPYING3.LIB). If not, see <http://www.gnu.org/licenses/>.
    
#include "patmos.h"
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#undef errno
extern int  errno;

// Keep the offset in stdin so that we can return the correct value for lseek
// and support SEEK_SET (fseek with SEEK_CUR will always convert to SEEK_SET)
// TODO I guess there should be a nicer way to do this ?!
int __patmos_stdin_offset = 0;

//******************************************************************************
/// _lseek - seek position for a file descriptor.
int _lseek(int fd, off_t offset, int whenceq)
{
  // For stdin and (positive) seek relative to current pos, we just consume bytes
  if (fd == STDIN_FILENO) {
    int curoff;

    switch (whenceq) {
    case SEEK_SET: curoff = offset - __patmos_stdin_offset; break;
    case SEEK_CUR: curoff = offset; break;
    default:
	// We do not support SEEK_END for now
	errno = EINVAL;
	return -1;
    }
    if (curoff < 0) {
	// We do not support seeking backwards
	errno = EINVAL;
	return -1;
    }

    int i;

    // consume data from UART
    for(i = 0; i < curoff; i++)
    {
      int s, c;

      // wait for data to be available from the UART
      do
      {
        __PATMOS_UART_STATUS(s);
      } while((s & (__PATMOS_UART_DAV | __PATMOS_UART_PAE)) == 0);

      // reached EOF?
      if ((s & __PATMOS_UART_PAE) == 0)
      {
        // read the data from the UART.
        __PATMOS_UART_RD_DATA(c);
      }
      else
      {
        // signal EOF
        errno = EINVAL;
        return -1;
      }
    }

    // successful, update offset
    __patmos_stdin_offset += curoff;

    return __patmos_stdin_offset; 
  }

  // TODO: implement for simulator target
  errno  = EBADF;
  return -1;
}

