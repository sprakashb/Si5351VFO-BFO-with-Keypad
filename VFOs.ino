//------------
void set_vfoA()             // 'A' key
{
  // key remains pressed > 2 sec: save freq
  if (LONG_PRESS)
  { store_freq();
    return;
  }
  VFO_mode = true;
  actvVFO = 'A';
  load_freq();
}

void set_vfoB()             // 'B' key
{
  // key remains pressed > 2 sec: save freq
  if (LONG_PRESS)
  { store_freq();
    return;
  }
  VFO_mode = true;
  actvVFO = 'B';
  load_freq();
}

void set_vfoC()             // 'C' key
{
  // key remains pressed > 2 sec: save freq
  if (LONG_PRESS)
  { store_freq();
    return;
  }
  VFO_mode = true;
  actvVFO = 'C';
  load_freq();
}

void set_vfoD()             // 'D' key
{
  // key remains pressed > 2 sec: save freq
  if (LONG_PRESS)
  { store_freq();
    return;
  }
  VFO_mode = true;
  actvVFO = 'D';
  load_freq();
}


