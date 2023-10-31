/*
 * analyze_self.idc -- Analyzes a SELF file, find it's TOC, OPD and import/export structures.
 *
 * Copyright (C) Youness Alaoui (KaKaRoTo)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 *
 */

#include "common.idh"

static FindOpd() {
  auto ea, seg, toc, next_toc, found_seg;

  found_seg = 0;

  for (seg = FirstSeg(); found_seg == 0 && NextSeg(seg) != seg; seg = NextSeg(seg)) {
    if (SegName(seg) == ".opd")
      found_seg = seg;
    for (ea = SegStart(seg) + 0x30; ea + 0x18 < SegEnd(seg); ea = ea + 0x18) {
      toc = Qword(ea + 0x08);
      next_toc = Qword(ea + 0x20);
      //Message("testing segment %X (%s) - %X - %X\n", ea, SegName(seg), toc, next_toc);
      if (toc == 0  || toc == 0xFFFFFFFFFFFFFFFF || toc != next_toc) {
	found_seg = 0;
	break;
      }
      if (ea - SegStart(seg) > 0x100)
        break;
      found_seg = seg;
    }
  }

  if (found_seg != 0) {
    RenameSeg(found_seg, ".opd");
    Message("Found Opd: 0x%X - TOC = 0x%X\n", found_seg, Qword(found_seg + 0x8));
  }

  return found_seg;
}

static FindToc(opd) {
  auto toc;

  toc = Qword(opd + 0x08);
  MakeName(toc, "TOC");
  return toc;
}

static FindImportsExports() {
  auto ea, seg, size, import_start, import_end, export_start, export_end;

  CreateImportStructure();
  CreateExportStructure();

  Message("Finding Import/Export structure\n");

  for (seg = FirstSeg(); export_start == 0 && NextSeg(seg) != seg; seg = NextSeg(seg)) {
    if ((SegEnd(seg) - SegStart(seg)) % 0x1C != 0)
      continue;

    for (ea = SegStart(seg); ea + 0x1c < SegEnd(seg); ea = ea + 0x1C) {
      size = Word(ea);
      if (size != 0x1C00) {
	export_start = 0;
	break;
      }
      export_start = seg;
    }
  }

  if (export_start == 0)
    return;

  export_end = SegEnd(export_start);
  RenameSeg(export_start, "Exports");
  Message("Found Export Table: 0x%X\n", export_start);

  for (seg = export_start; import_start == 0 && NextSeg(seg) != seg; seg = NextSeg(seg)) {
    if ((SegEnd(seg) - SegStart(seg)) % 0x2C != 0)
      continue;

    for (ea = SegStart(seg); ea + 0x2c < SegEnd(seg); ea = ea + 0x2C) {
      size = Word(ea);
      if (size != 0x2C00) {
	import_start = 0;
	break;
      }
      import_start = seg;
    }
  }

  if (import_start == 0)
    return;

  import_end = SegEnd(import_start);
  RenameSeg(import_start, "Imports");
  Message("Found Import Table: 0x%X\n", import_start);

  CreateExports(export_start, export_end);
  CreateImports(import_start, import_end);

  return ea;
}

static main() {
  auto ea, toc, opd, make_unk;

  make_unk = AskYN (0, "Do you want to undefine the entire database before continuing?\n"
         "It is recomended to start fresh because IDA can screw up the file otherwise.\n"
         "WARNING: You will loose any work you've done on this file!!");

  if (make_unk == -1) {
    Message("Canceled\n");
    return;
  }

  if (make_unk == 1)
    MakeUnknown(0, BADADDR, DOUNK_SIMPLE);
  opd = FindOpd();
  if (opd == 0) {
    Message("Could not find the OPD segment\n");
    return;
  }
  toc = FindToc(opd);

   if (toc != 0) {
    Message("\nFound TOC at 0x%X\n", toc);
    opd = CreateOpd64(opd, SegEnd(opd));
    FindImportsExports();
    Message("\TOC label at 0x%X\n", toc);
    Warning(form("%s\n%s\n%s 0x%X\n%s",
                 "Done.",
                 "Don't forget to go to Options->General->Analysis->"
                 "Processor specific options\n",
                 "And under TOC Address, enter : ", toc,
                 "Then press ok, then Reanalyze program"));
  } else {
    Message("Sorry, couldn't find the TOC");
  }
}
