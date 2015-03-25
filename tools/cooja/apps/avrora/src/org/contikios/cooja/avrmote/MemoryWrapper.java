/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.contikios.cooja.avrmote;

import avrora.sim.platform.memory.FlashMemory;
import java.io.IOException;
import java.io.RandomAccessFile;
import org.contikios.cooja.interfaces.flash.FlashMemoryAdapter;

/**
 *
 * @author comawill
 */
public class MemoryWrapper implements FlashMemoryAdapter {
  private final FlashMemory memory;

  public MemoryWrapper(FlashMemory flashMemory) {
    this.memory = flashMemory;
  }

  @Override
  public String getName() {
    return memory.getClass().getSimpleName();
  }

  @Override
  public int getNumberOfPages() {
    return memory.getNumberOfPages();
  }

  @Override
  public int getPageSize() {
    return memory.getPageSize();
  }

  @Override
  public void setNumberOfPages(int pages) {
    memory.setNumberOfPages(pages);
  }

  @Override
  public void setOutputStream(RandomAccessFile stream) throws IOException {
    memory.setOutputStream(stream);
  }

  @Override
  public void setPageSize(int size) {
    memory.setPageSize(size);
  }

}
