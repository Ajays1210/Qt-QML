import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Silences the editor warning about "itemModel" / "controller". These names are
// created in main.cpp with setContextProperty, so the editor cannot see them
// while you type, but they do exist at runtime.
// qmllint disable unqualified

ApplicationWindow {
  id: window

  width: 480
  height: 640
  visible: true
  title: "Location List"

  // Row number the user clicked in the list. -1 means nothing is selected.
  property int selectedIndex: -1

  ColumnLayout {
    anchors.fill: parent
    anchors.margins: 10
    spacing: 10

    // The list. "itemModel" is the C++ Model class, set in main.cpp.
    ListView {
      id: listView

      Layout.fillWidth: true
      Layout.fillHeight: true

      clip: true
      model: itemModel

      // One row per item. uniqueId, lat, longi, comment and index come from
      // the role names defined in Model::roleNames().
      delegate: ItemDelegate {
        width: listView.width
        highlighted: index === window.selectedIndex
        text: "ID " + uniqueId + "  (" + lat.toFixed(4) + ", " + longi.toFixed(4) + ")\n" + comment

        onClicked: window.selectedIndex = index
      }
    }

    RowLayout {
      Layout.fillWidth: true
      spacing: 10

      // ADD: open an empty form.
      Button {
        text: "Add"
        Layout.fillWidth: true

        onClicked: formPopup.openForm(false, "", "", "", "")
      }

      // UPDATE: open the form filled with the selected row.
      Button {
        text: "Update"
        Layout.fillWidth: true
        enabled: window.selectedIndex >= 0

        onClicked: {
          var item = itemModel.get(window.selectedIndex);
          formPopup.openForm(true, item.uniqueId, item.lat.toFixed(6),
                             item.longi.toFixed(6), item.comment);
        }
      }

      // DELETE: send the delete command. The row is removed later, only when
      // the server's UDP response arrives with Ack = success.
      Button {
        text: "Delete"
        Layout.fillWidth: true
        enabled: window.selectedIndex >= 0

        onClicked: {
          var item = itemModel.get(window.selectedIndex);
          controller.sendDelete(item.uniqueId);
          window.selectedIndex = -1;
        }
      }
    }
  }

  // The form window used for both Add and Update.
  Popup {
    id: formPopup

    modal: true
    focus: true
    anchors.centerIn: parent
    width: 300

    // false = Add, true = Update. Changes the title and locks the ID field.
    property bool isUpdateMode: false

    // Called by the Add and Update buttons: fills the fields, then shows the form.
    function openForm(update, id, lat, longi, comment) {
      isUpdateMode = update;
      idField.text = String(id);
      latitudeField.text = String(lat);
      longitudeField.text = String(longi);
      commentField.text = comment;
      open();
    }

    ColumnLayout {
      width: parent.width
      spacing: 8

      Label {
        text: formPopup.isUpdateMode ? "Update Item" : "Add Item"
        font.bold: true
      }

      Label { text: "Unique ID" }
      TextField {
        id: idField
        Layout.fillWidth: true
        enabled: !formPopup.isUpdateMode   // the ID cannot change in Update
        validator: IntValidator { bottom: 0 }
      }

      Label { text: "Latitude" }
      TextField {
        id: latitudeField
        Layout.fillWidth: true
        validator: DoubleValidator {}
      }

      Label { text: "Longitude" }
      TextField {
        id: longitudeField
        Layout.fillWidth: true
        validator: DoubleValidator {}
      }

      Label { text: "Comment" }
      TextField {
        id: commentField
        Layout.fillWidth: true
        maximumLength: 49   // struct field is char[50], 1 byte is kept for '\0'
      }

      RowLayout {
        Layout.fillWidth: true
        spacing: 8

        // APPLY: send the command over TCP. The list changes later, when the
        // server answers on UDP.
        Button {
          text: "Apply"
          Layout.fillWidth: true
          enabled: idField.acceptableInput
                   && latitudeField.acceptableInput
                   && longitudeField.acceptableInput

          onClicked: {
            var id = parseInt(idField.text);
            var lat = parseFloat(latitudeField.text);
            var longi = parseFloat(longitudeField.text);

            if (formPopup.isUpdateMode)
              controller.sendUpdate(id, lat, longi, commentField.text);
            else
              controller.sendAdd(id, lat, longi, commentField.text);

            formPopup.close();
          }
        }

        Button {
          text: "Cancel"
          Layout.fillWidth: true
          onClicked: formPopup.close()
        }
      }
    }
  }
}