#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lusb0_usb.h"

#define VND_GET_LEDS 0x10
#define VND_SET_LED  0x11

static usb_dev_handle *pDevH = NULL;  // Usb device handle
static struct usb_device *pDev;
static struct usb_bus *pBus;
static uint8_t leds;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  ConnectUSBDevice();
}

MainWindow::~MainWindow()
{
  DisconnectUSBDevice();
  delete ui;
}

void MainWindow::on_pushButton0_clicked( void )
{
  if ( leds & 1 )   VendorUniqueSetLedCmd( 0, 0 );
  else              VendorUniqueSetLedCmd( 0, 1 );
  
  UpdateLedsOnForm();
  ui->statusBar->showMessage( "Toggled Led 0" );
}

void MainWindow::on_pushButton1_clicked( void )
{
  if ( leds & 2 )   VendorUniqueSetLedCmd( 1, 0 );
  else              VendorUniqueSetLedCmd( 1, 1 );
  
  UpdateLedsOnForm();
  ui->statusBar->showMessage( "Toggled Led 1" );
}

void MainWindow::on_pushButton2_clicked( void )
{
  if ( leds & 4 )   VendorUniqueSetLedCmd( 2, 0 );
  else              VendorUniqueSetLedCmd( 2, 1 );
  
  UpdateLedsOnForm();
  ui->statusBar->showMessage( "Toggled Led 2" );
}

void MainWindow::on_pushButton3_clicked( void )
{
  if ( leds & 8 )   VendorUniqueSetLedCmd( 3, 0 );
  else              VendorUniqueSetLedCmd( 3, 1 );
  
  UpdateLedsOnForm();
  ui->statusBar->showMessage( "Toggled Led 3" );
}

void MainWindow::on_pushButton4_clicked( void )
{
  if ( leds & 16 )  VendorUniqueSetLedCmd( 4, 0 );
  else              VendorUniqueSetLedCmd( 4, 1 );
  
  UpdateLedsOnForm();
  ui->statusBar->showMessage( "Toggled Led 4" );
}

void MainWindow::VendorUniqueSetLedCmd( int ledNo, int on )
{
  if ( pDevH )
  {
    usb_control_msg( 
                  pDevH,            // Device handle
                  USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
                                    // bmRequestType
                  VND_SET_LED,      // bRequest
                  on,               // wValue
                  ledNo,            // wIndex
                  NULL,             // char *pBuffer
                  0x0000,           // wLength
                  1000 );           // Timeout (ms)
  }
}

uint8_t MainWindow::VendorUniqueGetLedsCmd( void )
{
  uint8_t leds;
  
  if ( pDevH )
  {
    usb_control_msg( 
                  pDevH,            // Device handle
                  USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
                                    // bmRequestType
                  VND_GET_LEDS,     // bRequest
                  0,                // wValue
                  0,                // wIndex
                  (char*)&leds,     // char *pBuffer
                  1,                // wLength
                  1000 );           // Timeout (ms)
  }
  
  return leds;
}

void MainWindow::UpdateLedsOnForm( void )
{
  leds = VendorUniqueGetLedsCmd();
  
  if ( leds & 1 )   ui->qLed0->setValue( true );
  else              ui->qLed0->setValue( false );
  if ( leds & 2 )   ui->qLed1->setValue( true );
  else              ui->qLed1->setValue( false );
  if ( leds & 4 )   ui->qLed2->setValue( true );
  else              ui->qLed2->setValue( false );
  if ( leds & 8 )   ui->qLed3->setValue( true );
  else              ui->qLed3->setValue( false );
  if ( leds & 16 )  ui->qLed4->setValue( true );
  else              ui->qLed4->setValue( false );
}

void MainWindow::ConnectUSBDevice( void )
{
  if ( pDevH == NULL )
  {
    usb_find_busses();
    usb_find_devices();
    
    // Enumerate USB devices ...
    for ( pBus = usb_get_busses(); pBus; pBus = pBus->next)
    {
      for ( pDev = pBus->devices; pDev; pDev = pDev->next)
      {
        if ((pDev->descriptor.idVendor  == 0x2544 ) &&
            (pDev->descriptor.idProduct == 0x0001 )    )
        {
          pDevH = usb_open( pDev );
          if ( pDevH )
          {
            if ( usb_set_configuration( 
                      pDevH, pDev->config->bConfigurationValue ) != 0 )
            {
              usb_close( pDevH );
              pDevH = NULL;
              goto found;
            }
            
            if ( usb_claim_interface( pDevH, 0 ) != 0 )
            {
              usb_close( pDevH );
              pDevH = NULL;
              goto found;
            }
          }
          goto found;
        }
      }
    }

found:
    if ( pDevH == NULL )
    {
      ui->statusBar->showMessage( "No USB device found" );
    }
    else
    {
      ui->statusBar->showMessage( "EFM32 Vendor Unique Device found" );
      leds = VendorUniqueGetLedsCmd();
      UpdateLedsOnForm();
    }
  }
}

void MainWindow::DisconnectUSBDevice( void )
{
  if ( pDevH )
  {
    usb_set_configuration( pDevH, 0 );
    usb_release_interface( pDevH, 0 );
    usb_close( pDevH );
    pDevH = NULL;
  }
}
