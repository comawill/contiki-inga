/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.contikios.cooja.interfaces;

import java.awt.BorderLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import org.apache.log4j.Priority;
import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.FlashMemoryMote;
import org.contikios.cooja.HasQuickHelp;
import org.contikios.cooja.Mote;
import org.contikios.cooja.MoteInterface;
import org.contikios.cooja.interfaces.flash.FlashMemory;
import org.jdom.Element;

/**
 * Visual interface for managing all connected Flash Memory Devices.
 *
 * @author Sebastian Willenborg
 */
@ClassDescription("FlashMemory")
public class FlashMemoryInterface extends MoteInterface implements HasQuickHelp {
  private final JPanel interfacePanel;
  private final FlashMemoryMote flashmote;
  private final Map<String, FlashMemoryPanel> memoryPanels = new HashMap<>();
  private final Map<String, String> memoryFiles = new HashMap<>();
  private static final org.apache.log4j.Logger logger = org.apache.log4j.Logger.getLogger(FlashMemoryInterface.class);

  public FlashMemoryInterface(Mote mote) {
    if (mote != null && FlashMemoryMote.class.isAssignableFrom(mote.getClass())) {
      flashmote = (FlashMemoryMote) mote;
    } else {
      throw new RuntimeException("Mote is not a FlashMemoryMote");
    }

    interfacePanel = new JPanel();
    interfacePanel.setLayout(new BoxLayout(interfacePanel, BoxLayout.Y_AXIS));
    FlashMemory[] memoryDevices = flashmote.getFlashMemory();
    for (FlashMemory memoryDevice : memoryDevices) {


      FlashMemoryPanel memoryPanel = new FlashMemoryPanel(((Mote) flashmote).getSimulation().getCooja(), memoryDevice);
      memoryPanels.put(memoryDevice.getName(), memoryPanel);
      interfacePanel.add(memoryPanel);

      /* Add separator if not last item */
      if (!memoryDevices[memoryDevices.length - 1].equals(memoryDevice)) {
        interfacePanel.add(new JSeparator());
      }
    }
  }

  @Override
  public JPanel getInterfaceVisualizer() {
    return interfacePanel;
  }

  @Override
  public void releaseInterfaceVisualizer(JPanel panel) {
  }

  @Override
  public Collection<Element> getConfigXML() {
    List<Element> elements = new LinkedList<>();
    Element memoryElement;
    for (FlashMemory memoryDevice : flashmote.getFlashMemory()) {
      memoryElement = new Element("flashmemory");
      memoryElement.addContent(memoryDevice.getName());
      if (memoryFiles.containsKey(memoryDevice.getName())) {
        memoryElement.setAttribute("file", memoryFiles.get(memoryDevice.getName()));
      }
      memoryElement.setAttribute("pages", "" + memoryDevice.getNumberOfPages());
      memoryElement.setAttribute("pagesize", "" + memoryDevice.getPageSize());
      elements.add(memoryElement);
    }
    return elements;
  }

  @Override
  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (!element.getName().equals("flashmemory")) {
        continue;
      }
      String memoryName = element.getTextTrim();
      FlashMemory memoryDevice = null;
      for (FlashMemory m : flashmote.getFlashMemory()) {
        if (m.getName().equals(memoryName)) {
          memoryDevice = m;
          break;
        }
      }
      if (memoryDevice == null) {
        logger.error("Memory " + memoryName + " was not found and will not be configured");
        continue;
      }
      try {
        if (element.getAttribute("pages") != null) {
          memoryDevice.setNumberOfPages(Integer.parseInt(element.getAttributeValue("pages")));
        }
        if (element.getAttribute("pagesize") != null) {
          memoryDevice.setPageSize(Integer.parseInt(element.getAttributeValue("pagesize")));
        }
        if (element.getAttribute("file") != null) {
          String filename = element.getAttributeValue("file");
          memoryFiles.put(memoryName, filename);
          memoryDevice.setOutputFile(new File(filename));
        }

      } catch (IOException ex) {
        logger.log(Priority.ERROR, null, ex);
      }
    }
    for (FlashMemoryPanel panel : memoryPanels.values()) {
      panel.updateMemoryInfo();
    }
  }

  @Override
  public String getQuickHelp() {
    String help = "";
    help += "<p><b>" + Cooja.getDescriptionOf(this) + "</b>";
    help += "<p>Lists available flash memory devices and allows to select image files.";

    return help;
  }


  private class FlashMemoryPanel extends JPanel {
    private final FlashMemory memory;
    private final JButton configureButton;
    private final JLabel pathLabel;

    public FlashMemoryPanel(Cooja gui, FlashMemory memory) {
      this.memory = memory;
      setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
      setBorder(BorderFactory.createEmptyBorder(3, 3, 3, 3));
      JPanel sensorSubPanel = new JPanel();
      sensorSubPanel.setLayout(new BorderLayout());
      JLabel namelabel = new JLabel(memory.getName());
      sensorSubPanel.add(BorderLayout.CENTER, namelabel);
      Font f = namelabel.getFont();
      namelabel.setFont(f.deriveFont(f.getStyle() | Font.BOLD));



      configureButton = new JButton("--");
      sensorSubPanel.add(BorderLayout.EAST, configureButton);
      add(sensorSubPanel);

      JPanel pathPanel = new JPanel(new BorderLayout());

      pathLabel = new JLabel("--");
      pathPanel.add(BorderLayout.WEST, pathLabel);
      add(BorderLayout.CENTER, pathPanel);

      JPanel feederPanel = new JPanel(new BorderLayout());
      feederPanel.add(BorderLayout.WEST, new JLabel("Page Size:"));
      JLabel feederDescLabel = new JLabel("" + memory.getPageSize());
      //setFeederDescription();
      feederPanel.add(feederDescLabel);
      add(BorderLayout.CENTER, feederPanel);

      JPanel feederPanel2 = new JPanel(new BorderLayout());
      feederPanel2.add(BorderLayout.WEST, new JLabel("# of Pages:"));
      JLabel feederDescLabel2 = new JLabel("" + memory.getNumberOfPages());
      //setFeederDescription();
      feederPanel2.add(feederDescLabel2);
      add(BorderLayout.CENTER, feederPanel2);

      configureButton.addActionListener(new ActionListener() {

        @Override
        public void actionPerformed(ActionEvent e) {
          if (!memoryFiles.containsKey(memory.getName())) {
            JFileChooser chooser = new JFileChooser();
            if (chooser.showOpenDialog(null) == JFileChooser.APPROVE_OPTION) {
              File newfile = chooser.getSelectedFile();
              try {
                memory.setOutputFile(newfile);
                String filename = newfile.getPath();
                pathLabel.setText(filename);
                memoryFiles.put(memory.getName(), filename);
              } catch (IOException ex) {
                Logger.getLogger(FlashMemoryInterface.class.getName()).log(Level.SEVERE, null, ex);
              }
            }

          } else {
            memoryFiles.remove(memory.getName());
          }
          updateMemoryInfo();
        }
      });
    }

    void updateMemoryInfo() {
      if (memoryFiles.containsKey(memory.getName())) {
        configureButton.setText("Remove Image");
        pathLabel.setText(memoryFiles.get(memory.getName()));
      } else {
        configureButton.setText("Set Image");
        pathLabel.setText("No Image set");
      }
    }
  }
}