import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
  id: window

  width: 480
  height: 640
  visible: true
  title: "Location List"

  property int selectedIndex: -1

  ColumnLayout {
    anchors.fill: parent
    anchors.margins: 10
    spacing: 10

    ListView {
      id: listView

      Layout.fillWidth: true
      Layout.fillHeight: true

      clip: true
      model: itemModel

      delegate: ItemDelegate {
        width: listView.width
        highlighted: ListView.isCurrentItem
        text: "ID " + uniqueId + "  (" + Number(lat).toFixed(4) + ", " + Number(longi).toFixed(4) + ")\n" + comment

        onClicked: {
          listView.currentIndex = index;
          window.selectedIndex = index;
        }
      }
    }

    RowLayout {
      Layout.fillWidth: true
      spacing: 10

      Button {
        text: "Add"
        Layout.fillWidth: true

        onClicked: {
          formPopup.isUpdateMode = false;
          formPopup.uniqueIdField.text = "";
          formPopup.latField.text = "";
          formPopup.longiField.text = "";
          formPopup.commentField.text = "";
          formPopup.open();
        }
      }

      Button {
        text: "Update"
        Layout.fillWidth: true
        enabled: window.selectedIndex >= 0

        onClicked: {
          var item = itemModel.get(window.selectedIndex);
          formPopup.isUpdateMode = true;
          formPopup.uniqueIdField.text = String(item.uniqueId);
          formPopup.latField.text = String(item.lat);
          formPopup.longiField.text = String(item.longi);
          formPopup.commentField.text = item.comment;
          formPopup.open();
        }
      }

      Button {
        text: "Delete"
        Layout.fillWidth: true
        enabled: window.selectedIndex >= 0

        onClicked: {
          var item = itemModel.get(window.selectedIndex);
          controller.sendDelete(item.uniqueId);
          window.selectedIndex = -1;
          listView.currentIndex = -1;
        }
      }
    }
  }

  Popup {
    id: formPopup

    modal: true
    focus: true
    anchors.centerIn: parent
    width: 300

    property bool isUpdateMode: false

    property alias uniqueIdField: idField
    property alias latField: latitudeField
    property alias longiField: longitudeField
    property alias commentField: commentField

    ColumnLayout {
      width: parent.width
      spacing: 8

      Label {
        text: formPopup.isUpdateMode ? "Update Item" : "Add Item"
        font.bold: true
      }

      Label {
        text: "Unique ID"
      }
      TextField {
        id: idField
        Layout.fillWidth: true
        enabled: !formPopup.isUpdateMode
        validator: IntValidator {
          bottom: 0
        }
      }

      Label {
        text: "Latitude"
      }
      TextField {
        id: latitudeField
        Layout.fillWidth: true
        validator: DoubleValidator {}
      }

      Label {
        text: "Longitude"
      }
      TextField {
        id: longitudeField
        Layout.fillWidth: true
        validator: DoubleValidator {}
      }

      Label {
        text: "Comment"
      }
      TextField {
        id: commentField
        Layout.fillWidth: true
        maximumLength: 49
      }

      RowLayout {
        Layout.fillWidth: true
        spacing: 8

        Button {
          text: "Apply"
          Layout.fillWidth: true
          enabled: idField.text.length > 0

          onClicked: {
            var id = parseInt(idField.text);
            var lat = parseFloat(latitudeField.text);
            var longi = parseFloat(longitudeField.text);

            if (isNaN(id) || isNaN(lat) || isNaN(longi))
              return;

            if (formPopup.isUpdateMode) {
              controller.sendUpdate(id, lat, longi, commentField.text);
            } else {
              controller.sendAdd(id, lat, longi, commentField.text);
            }

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
