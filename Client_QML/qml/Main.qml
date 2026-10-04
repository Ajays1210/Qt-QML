import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Silences the editor warning about "itemModel" / "controller". These names are
// created in main.cpp with setContextProperty, so the editor cannot see them
// while you type, but they do exist at runtime.
// qmllint disable unqualified

ApplicationWindow {
  id: window

  width: 920
  height: 620
  visible: true
  title: "Location Manager"
  color: "#1c1d1f"

  // Row number the user clicked in the table. -1 means nothing is selected.
  property int selectedIndex: -1

  // ---- Theme: every colour and size lives here, so the look is easy to change ----
  readonly property color accent: "#e8ff3a"        // lime highlight (buttons, focus, ticks)
  readonly property color panelColor: "#2a2b2d"    // side panel background
  readonly property color headerColor: "#26272a"   // table header
  readonly property color rowColor: "#222325"      // normal row
  readonly property color hoverColor: "#2d2f32"    // row under the mouse
  readonly property color selectColor: "#3d4420"   // selected row (olive tint)
  readonly property color gridColor: "#34363a"     // cell border lines
  readonly property color textColor: "#d9dbde"
  readonly property color mutedColor: "#8b9096"
  readonly property int rowHeight: 40

  // Column widths, shared by the header and every row so the grid lines up.
  // The Comment column takes whatever width is left.
  readonly property int checkWidth: 52
  readonly property int idWidth: 120
  readonly property int latWidth: 140
  readonly property int longWidth: 140
  readonly property int commentWidth: Math.max(160, tableView.width - checkWidth - idWidth - latWidth - longWidth)

  // ---- Reusable pieces ----

  // One grid cell: a rectangle with a thin border and a text label inside.
  component Cell: Rectangle {
    property alias text: cellLabel.text
    property alias bold: cellLabel.font.bold
    property alias textColor: cellLabel.color

    height: window.rowHeight
    border.color: window.gridColor
    border.width: 1

    Label {
      id: cellLabel
      anchors.fill: parent
      anchors.leftMargin: 12
      anchors.rightMargin: 12
      verticalAlignment: Text.AlignVCenter
      elide: Text.ElideRight
      color: window.textColor
      font.pixelSize: 13
    }
  }

  // A flat button. primary = lime filled (main action), otherwise dark grey.
  component ActionButton: Button {
    id: control
    property bool primary: false

    implicitHeight: 36
    implicitWidth: 96

    contentItem: Label {
      text: control.text
      font.bold: true
      font.pixelSize: 12
      font.capitalization: Font.AllUppercase
      horizontalAlignment: Text.AlignHCenter
      verticalAlignment: Text.AlignVCenter
      color: !control.enabled ? "#5f6368" : (control.primary ? "#1b1c1e" : window.textColor)
    }
    background: Rectangle {
      radius: 4
      color: !control.enabled ? "#262729" : control.primary ? (control.hovered ? "#f3ff7a" : window.accent) : (control.hovered ? "#3c3e42" : "#303236")
    }
  }

  // A text field with only an underline (turns lime while typing).
  component DarkField: TextField {
    color: enabled ? "#ffffff" : "#7a7f85"
    selectionColor: window.accent
    selectedTextColor: "#1b1c1e"
    font.pixelSize: 14
    leftPadding: 0
    background: Rectangle {
      color: "transparent"
      Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 2
        color: parent.parent.activeFocus ? window.accent : "#5a5d62"
      }
    }
  }

  // Opens the side panel. update = false -> Add (empty), true -> Update (filled).
  // Called by the Add and Update buttons.
  function openPanel(update, id, lat, longi, comment) {
    sidePanel.isUpdateMode = update;
    idField.text = String(id);
    latitudeField.text = String(lat);
    longitudeField.text = String(longi);
    commentField.text = comment;
    sidePanel.open();
  }

  // ======================= MAIN SCREEN =======================
  ColumnLayout {
    anchors.fill: parent
    anchors.margins: 20
    spacing: 14

    // ---- Title ----
    ColumnLayout {
      spacing: 2

      Label {
        text: "Location Manager"
        color: "#ffffff"
        font.pixelSize: 26
      }
      Label {
        text: tableView.count + (tableView.count === 1 ? " record" : " records")
        color: window.mutedColor
        font.pixelSize: 12
      }
    }

    // ---- Toolbar: search on the left, actions on the right ----
    RowLayout {
      Layout.fillWidth: true
      spacing: 10

      TextField {
        id: searchField
        Layout.preferredWidth: 340
        placeholderText: "Search by ID or comment"
        placeholderTextColor: window.mutedColor
        color: "#ffffff"
        font.pixelSize: 13
        leftPadding: 16
        selectionColor: window.accent
        selectedTextColor: "#1b1c1e"

        // Typing in the search box clears the selection, so Update/Delete can
        // never act on a row that has just been hidden.
        onTextChanged: window.selectedIndex = -1

        background: Rectangle {
          radius: 18
          color: "#2a2b2d"
          border.color: searchField.activeFocus ? window.accent : "#3a3c40"
        }
      }

      Item {
        Layout.fillWidth: true
      }   // pushes the buttons to the right

      // ADD: open the side panel empty.
      ActionButton {
        text: "+ Add"
        primary: true
        onClicked: window.openPanel(false, "", "", "", "")
      }

      // UPDATE: open the side panel filled with the selected row.
      ActionButton {
        text: "Update"
        enabled: window.selectedIndex >= 0
        onClicked: {
          var item = itemModel.get(window.selectedIndex);
          window.openPanel(true, item.uniqueId, item.lat.toFixed(6), item.longi.toFixed(6), item.comment);
        }
      }

      // DELETE: send the delete command. The row is removed later, only when
      // the server's UDP response arrives with Ack = success.
      ActionButton {
        text: "Delete"
        enabled: window.selectedIndex >= 0
        onClicked: {
          var item = itemModel.get(window.selectedIndex);
          controller.sendDelete(item.uniqueId);
          window.selectedIndex = -1;
        }
      }
    }

    // ---- Table header: one fixed row of column titles ----
    Row {
      Layout.fillWidth: true
      Layout.bottomMargin: -14   // cancels the layout gap so the body touches the header
      spacing: -1                // overlap borders so the lines stay 1px thick

      Cell {
        width: window.checkWidth
        color: window.headerColor
      }
      Cell {
        width: window.idWidth
        color: window.headerColor
        text: "UNIQUE ID"
        bold: true
        textColor: window.mutedColor
      }
      Cell {
        width: window.latWidth
        color: window.headerColor
        text: "LATITUDE"
        bold: true
        textColor: window.mutedColor
      }
      Cell {
        width: window.longWidth
        color: window.headerColor
        text: "LONGITUDE"
        bold: true
        textColor: window.mutedColor
      }
      Cell {
        width: window.commentWidth
        color: window.headerColor
        text: "COMMENT"
        bold: true
        textColor: window.mutedColor
      }
    }

    // ---- Table body: one grid row per item. "itemModel" is the C++ Model
    // class (set in main.cpp). uniqueId, lat, longi, comment and index come
    // from the role names defined in Model::roleNames(). ----
    ListView {
      id: tableView

      Layout.fillWidth: true
      Layout.fillHeight: true

      clip: true
      model: itemModel

      delegate: Item {
        id: rowItem

        // Search filter: a row that does not match is hidden (height 0).
        readonly property bool matches: {
          var q = searchField.text.trim().toLowerCase();
          return q === "" || String(uniqueId).indexOf(q) >= 0 || comment.toLowerCase().indexOf(q) >= 0;
        }
        readonly property bool selected: index === window.selectedIndex

        width: tableView.width
        visible: matches
        height: matches ? window.rowHeight - 1 : 0   // -1 makes neighbouring borders overlap

        // Cell background: selected > mouse-over > normal.
        readonly property color cellColor: selected ? window.selectColor : (rowHover.hovered ? window.hoverColor : window.rowColor)

        HoverHandler {
          id: rowHover
        }

        Row {
          spacing: -1

          // First column: a small tick box that shows which row is selected.
          Cell {
            width: window.checkWidth
            color: rowItem.cellColor

            Rectangle {
              anchors.centerIn: parent
              width: 18
              height: 18
              radius: 3
              color: rowItem.selected ? window.accent : "transparent"
              border.color: rowItem.selected ? window.accent : "#6b7075"
              border.width: 2

              Label {
                anchors.centerIn: parent
                text: "✓"
                visible: rowItem.selected
                color: "#1b1c1e"
                font.bold: true
                font.pixelSize: 12
              }
            }
          }
          Cell {
            width: window.idWidth
            color: rowItem.cellColor
            text: uniqueId
          }
          Cell {
            width: window.latWidth
            color: rowItem.cellColor
            text: lat.toFixed(6)
          }
          Cell {
            width: window.longWidth
            color: rowItem.cellColor
            text: longi.toFixed(6)
          }
          Cell {
            width: window.commentWidth
            color: rowItem.cellColor
            text: comment
          }
        }

        // Clicking a row selects it. Clicking the selected row again deselects it.
        MouseArea {
          anchors.fill: parent
          onClicked: window.selectedIndex = (window.selectedIndex === index) ? -1 : index
        }
      }
    }
  }

  // ======================= SIDE PANEL (Add / Update) =======================
  // Slides in from the right edge. One panel is used for both Add and Update.
  Drawer {
    id: sidePanel

    edge: Qt.RightEdge
    width: 360
    height: window.height
    modal: true
    interactive: false   // only the buttons open/close it, not swiping

    // false = Add, true = Update. Changes the title and locks the ID field.
    property bool isUpdateMode: false

    background: Rectangle {
      color: window.panelColor
    }

    ColumnLayout {
      anchors.fill: parent
      anchors.margins: 24
      spacing: 14

      Label {
        text: sidePanel.isUpdateMode ? "Update Entry" : "Add New Entry"
        color: "#ffffff"
        font.pixelSize: 18
        font.bold: true
      }

      Label {
        text: "Unique ID"
        color: window.mutedColor
        font.pixelSize: 11
        Layout.topMargin: 8
      }
      DarkField {
        id: idField
        Layout.fillWidth: true
        enabled: !sidePanel.isUpdateMode   // the ID cannot change in Update
        validator: IntValidator {
          bottom: 0
        }
      }

      Label {
        text: "Latitude"
        color: window.mutedColor
        font.pixelSize: 11
      }
      DarkField {
        id: latitudeField
        Layout.fillWidth: true
        validator: DoubleValidator {}
      }

      Label {
        text: "Longitude"
        color: window.mutedColor
        font.pixelSize: 11
      }
      DarkField {
        id: longitudeField
        Layout.fillWidth: true
        validator: DoubleValidator {}
      }

      Label {
        text: "Comment"
        color: window.mutedColor
        font.pixelSize: 11
      }
      DarkField {
        id: commentField
        Layout.fillWidth: true
        maximumLength: 49   // struct field is char[50], 1 byte is kept for '\0'
      }

      Item {
        Layout.fillHeight: true
      }   // pushes the buttons to the bottom

      RowLayout {
        Layout.fillWidth: true
        spacing: 10

        Item {
          Layout.fillWidth: true
        }

        ActionButton {
          text: "Cancel"
          onClicked: sidePanel.close()
        }

        // APPLY: send the command over TCP. The table changes later, when the
        // server answers on UDP.
        ActionButton {
          text: sidePanel.isUpdateMode ? "Save" : "Create"
          primary: true
          enabled: idField.acceptableInput && latitudeField.acceptableInput && longitudeField.acceptableInput

          onClicked: {
            var id = parseInt(idField.text);
            var lat = parseFloat(latitudeField.text);
            var longi = parseFloat(longitudeField.text);

            if (sidePanel.isUpdateMode)
              controller.sendUpdate(id, lat, longi, commentField.text);
            else
              controller.sendAdd(id, lat, longi, commentField.text);

            sidePanel.close();
          }
        }
      }
    }
  }
}
