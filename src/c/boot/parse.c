/**
 * @file parse.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is part of Arctan.
 *
 * Arctan is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @DESCRIPTION
*/
#include <boot/parse.h>
#include <boot/mb2.h>
#include <global.h>

int Arc_ParseBootInfo() {
	ARC_DEBUG(INFO, "Parsing boot information\n");

	switch (Arc_BootMeta->boot_proc) {
	case ARC_BOOTPROC_MB2: {
		Arc_ParseMB2I();
		break;
	}
	}

	ARC_DEBUG(INFO, "Finished parsing boot information\n");

	return 0;
}
