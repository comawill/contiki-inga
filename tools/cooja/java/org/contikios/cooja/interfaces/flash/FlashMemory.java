/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.contikios.cooja.interfaces.flash;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import org.contikios.cooja.Mote;


/**
 *
 * @author comawill
 */
public class FlashMemory {
  private final Mote mote;
  private final FlashMemoryAdapter memory;

  public FlashMemory(Mote mote, FlashMemoryAdapter memory) {
    this.mote = mote;
    this.memory = memory;
  }

  /**
   * Returns the mote this memory device belongs to.
   *
   * @return Mote that owns this memory device.
   */
  public Mote getMote() {
    return mote;
  }

  /**
   * Returns name of this memory device.
   *
   * @return Name
   */
  public String getName() {
    return memory.getName();
  }

  public int getNumberOfPages() {
    return memory.getNumberOfPages();
  }

  public int getPageSize() {
    return memory.getPageSize();
  }

  public void setPageSize(int size) {
    memory.setPageSize(size);
  }

  public void setNumberOfPages(int number) {
    memory.setNumberOfPages(number);
  }

  public void setOutputFile(File file) throws IOException {
    if (file == null) {
      memory.setOutputStream(null);
    } else {
      memory.setOutputStream(new RandomAccessFile(file.getPath(), "rw"));
    }
  }
}
