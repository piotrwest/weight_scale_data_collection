#include "SimpleSdCard.h"

#include "SdFat.h"
#include "sdios.h"
#include "FreeStack.h"

SimpleSdCard::SimpleSdCard()
{
}

void SimpleSdCard::safeBegin()
{
  // use uppercase in hex and use 0X base prefix
  cout << uppercase << showbase << endl;

  if (!sd.begin(SD_CONFIG))
  {
    sd.initErrorHalt(&Serial);
  }
  if (sd.fatType() == FAT_TYPE_EXFAT)
  {
    cout << F("Type is exFAT") << endl;
  }
  else
  {
    cout << F("Type is FAT") << int(sd.fatType()) << endl;
  }

  cout << F("Card size: ") << sd.card()->sectorCount() * 512E-9;
  cout << F(" GB (GB = 1E9 bytes)") << endl;
  cidDmp();
}

void SimpleSdCard::cidDmp()
{
  cid_t cid;
  if (!sd.card()->readCID(&cid))
  {
    error("readCID failed");
  }
  cout << F("\nManufacturer ID: ");
  cout << hex << int(cid.mid) << dec << endl;
  cout << F("OEM ID: ") << cid.oid[0] << cid.oid[1] << endl;
  cout << F("Product: ");
  for (uint8_t i = 0; i < 5; i++)
  {
    cout << cid.pnm[i];
  }
  cout << F("\nVersion: ");
  cout << int(cid.prv_n) << '.' << int(cid.prv_m) << endl;
  cout << F("Serial number: ") << hex << cid.psn << dec << endl;
  cout << F("Manufacturing date: ");
  cout << int(cid.mdt_month) << '/';
  cout << (2000 + cid.mdt_year_low + 10 * cid.mdt_year_high) << endl;
  cout << endl;
}

#if SD_FAT_TYPE == 0
File * SimpleSdCard::getFile()
{
  return &file;
}
#elif SD_FAT_TYPE == 1
File32 * SimpleSdCard::getFile()
{
  return &file;
}
#elif SD_FAT_TYPE == 2
ExFile * SimpleSdCard::getFile()
{
  return &file;
}
#elif SD_FAT_TYPE == 3
FsFile * SimpleSdCard::getFile()
{
  return &file;
}
#else // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif // SD_FAT_TYPE