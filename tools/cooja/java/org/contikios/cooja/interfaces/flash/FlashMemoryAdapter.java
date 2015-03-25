/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.contikios.cooja.interfaces.flash;

import java.io.IOException;
import java.io.RandomAccessFile;

/**
 *
 * @author comawill
 */
public interface FlashMemoryAdapter {

  /**
   * Returns name of memory device.
   *
   * @return
   */
  String getName();

  int getNumberOfPages();

  int getPageSize();

  void setNumberOfPages(int pages);

  void setOutputStream(RandomAccessFile stream) throws IOException;

  void setPageSize(int size);
}
