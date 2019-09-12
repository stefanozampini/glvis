// Copyright (c) 2010, Lawrence Livermore National Security, LLC. Produced at
// the Lawrence Livermore National Laboratory. LLNL-CODE-443271. All Rights
// reserved. See file COPYRIGHT for details.
//
// This file is part of the GLVis visualization tool and library. For more
// information and source code availability see http://glvis.org.
//
// GLVis is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License (as published by the Free
// Software Foundation) version 2.1 dated February 1999.

#ifndef GLVIS_COLOR_PALETTES
#define GLVIS_COLOR_PALETTES

extern int RGB_Palette_Size;
extern double * RGB_Palette;

void Init_Palettes();
void Set_Palette (int num);
void Next_RGB_Palette();
void Prev_RGB_Palette();
int Select_New_RGB_Palette();

#endif
