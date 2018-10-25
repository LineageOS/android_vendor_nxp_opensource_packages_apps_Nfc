/******************************************************************************
 *  Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
 *  Not a Contribution.
 *
 *  Copyright (C) 2011-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  The original Work has been changed by NXP Semiconductors.
 *
 *  Copyright (C) 2015 The Android Open Source Project
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

 /**
  * @file phNxpConfig.cpp
  * @date 24 Aug 2016
  * @brief File containing code for dynamic selection of config files based on target.
  *
  * The target device has to be configured with some primary setting while booting.So a
  * config file will be picked while the target is booted. Here based on the target device
  * a configuration file will be selected dynamically and the device will be configured.
  */

#define LOG_TAG "pn54x"

#include <phNxpConfig.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <sys/stat.h>
#include <stdlib.h>

#include <phNxpLog.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <errno.h>
#include "sparse_crc32.h"
#if GENERIC_TARGET
const char alternative_config_path[] = "/data/nfc/";
#else
const char alternative_config_path[] = "";
#endif

#if 1
const char* transport_config_paths[] = {"/odm/etc/", "/vendor/etc/", "/etc/"};
#if(NXP_EXTNS == TRUE)
const char transit_config_path[] = "/data/nfc/";
#endif
#else
const char* transport_config_paths[] = {"res/"};
#endif
const int transport_config_path_size =
        (sizeof(transport_config_paths) / sizeof(transport_config_paths[0]));

/**
 *  @brief defines the different config files used.
 */

#define config_name             "libnfc-nxp.conf"
#if(NXP_EXTNS == TRUE)
#define extra_config_base       "libnfc-"
#else
#define extra_config_base       "libnfc-nxp-"
#endif

#define extra_config_ext        ".conf"
#define IsStringValue           0x80000000

const char config_timestamp_path[]    = "/data/nfc/libnfc-nxpConfigState.bin";
const char default_nxp_config_path[]  = "/etc/libnfc-nxp.conf";
const char rf_config_timestamp_path[] = "/data//nfc/libnfc-nxpRFConfigState.bin";
const char tr_config_timestamp_path[] = "/data//nfc/libnfc-nxpTransitConfigState.bin";
const char nxp_rf_config_path[]       = "/system/libnfc-nxp_RF.conf";

namespace {

size_t readConfigFile(const char* fileName, uint8_t** p_data) {
  FILE* fd = fopen(fileName, "rb");
  if (fd == nullptr) return 0;

  fseek(fd, 0L, SEEK_END);
  const size_t file_size = ftell(fd);
  rewind(fd);

  uint8_t* buffer = new uint8_t[file_size];
  size_t read = fread(buffer, file_size, 1, fd);
  fclose(fd);

  if (read == 1) {
    *p_data = buffer;
    return file_size;
  }

  return 0;
}

}  // namespace

/**
 *  @brief target platform ID values.
 */

typedef enum
{
  CONFIG_GENERIC                         = 0x00,
  MTP_TYPE_DEFAULT                       = 0x01, /**< default MTP config. DC DC ON */
  QRD_TYPE_DEFAULT                       = 0x02, /**< default QRD config DC DC OFF */
  MTP_TYPE_1                             = 0x03, /**< mtp config type1 : newer chip */
  MTP_TYPE_2                             = 0x04, /**< mtp config type2 TBD */
  QRD_TYPE_1                             = 0x05, /**< qrd config type1 DC DC ON*/
  QRD_TYPE_2                             = 0x06, /**< qrd config type2  Newer chip */
  MTP_TYPE_NQ3XX                         = 0x07, /**< qrd config : for NQ3XX chip */
  QRD_TYPE_NQ3XX                         = 0x08, /**< qrd config : for NQ3XX chip */
  MTP_TYPE_NQ4XX                         = 0x09, /**< mtp config : for NQ4XX chip */
  QRD_TYPE_NQ4XX                         = 0x10, /**< qrd config : for NQ4XX chip */
  DEFAULT_CONFIG                         = QRD_TYPE_DEFAULT, /**< default is qrd default config */
  CONFIG_INVALID                         = 0xFF
} CONFIGIDVALUE;

/**
 *  @brief Defines the values for different types of targets.
 */

typedef enum
{
  TARGET_GENERIC                       = 0x00, /**< new targets */
  TARGET_MSM8952                       = 264, /**< 8952 target */
  TARGET_MSM8976                       = 278, /**< 8976 target */
  TARGET_MSM8937                       = 294, /**< 8937 target */
  TARGET_MSM8953                       = 293, /**< 8953 target */
  TARGET_MSM8996                       = 246, /**< 8996 target*/
  TARGET_MSM8909                       = 245, /**< 8909w target */
  TARGET_MSM8998                       = 292, /**< 8998 target */
  TARGET_MSM8997                       = 306, /**< 8997 target */
  TARGET_MSM8917                       = 303, /**< 8997 target */
  TARGET_MSM8940                       = 313, /**< 8997 target */
  TARGET_SDM660                        = 317, /**< SDM660 target */
  TARGET_SDM630                        = 318, /**< SDM630 target */
  TARGET_SDM632                        = 349, /**< SDM632 target */
  TARGET_SDM439                        = 353, /**< SDM439 target */
  TARGET_SDM429                        = 354, /**< SDM429 target */
  TARGET_SDM845                        = 321, /**< SDM845 target */
  TARGET_SDM450                        = 338, /**< SDM450 target */
  TARGET_DEFAULT                       = TARGET_GENERIC, /**< new targets */
  TARGET_INVALID                       = 0xFF
} TARGETTYPE;

using namespace::std;

void findConfigFilePathFromTransportConfigPaths(const string& configName, string& filePath);
void readOptionalConfig(const char* optional);

class CNxpNfcParam : public string
{
public:
    CNxpNfcParam ();
    CNxpNfcParam (const char* name, const string& value);
    CNxpNfcParam (const char* name, unsigned long value);
    virtual ~CNxpNfcParam ();
    unsigned long numValue () const {return m_numValue;}
    const char* str_value () const {return m_str_value.c_str();}
    size_t str_len () const {return m_str_value.length();}
private:
    string          m_str_value;
    unsigned long   m_numValue;
};

class CNxpNfcConfig : public vector<const CNxpNfcParam*> {
 public:
  virtual ~CNxpNfcConfig();
  static CNxpNfcConfig& GetInstance();
  friend void readOptionalConfig(const char* optional);
  bool isModified();
  bool isModified(const char* pName);
  void resetModified();

  bool getValue(const char* name, char* pValue, size_t len) const;
  bool getValue(const char* name, unsigned long& rValue) const;
  bool getValue(const char* name, unsigned short& rValue) const;
  bool getValue(const char* name, char* pValue, long len, long* readlen) const;
  const CNxpNfcParam* find(const char* p_name) const;
  void readNxpTransitConfig(const char* fileName) const;
  void readNxpRFConfig(const char* fileName) const;
  void clean();

 private:
  CNxpNfcConfig();
  bool readConfig(const char* name, bool bResetContent);
  int     file_exist (const char* filename);
  int     getconfiguration_id (char * config_file);
  void moveFromList();
  void moveToList();
  void add(const CNxpNfcParam* pParam);
  void dump();
  bool isAllowed(const char* name);
  list<const CNxpNfcParam*> m_list;
  bool mValidFile;
  uint32_t config_crc32_;
  uint32_t config_crc32_rf_;
  uint32_t config_crc32_tr_;
  bool    mDynamConfig;
  string mCurrentFile;
  unsigned long m_timeStamp;
  unsigned long state;

  inline bool Is(unsigned long f) { return (state & f) == f; }
  inline void Set(unsigned long f) { state |= f; }
  inline void Reset(unsigned long f) { state &= ~f; }
};

/**
 * @brief This function reads the hardware information from the given path.
 *
 * This function receives the path and then reads the hardware information
 * from the file present in the given path. It reads the details like whether
 * it is QRD or MTP. It reads the data from that file and stores in buffer.
 * It also receives a count which tells the number of characters to be read
 * Finally the length of the buffer is returned.
 *
 * @param path The path where the file containing hardware details to be read.
 * @param buff The hardware details that is read from that path will be stored here.
 * @param count It represents the number of characters to be read from that file.
 * @return It returns the length of the buffer.
 */

static int read_line_from_file(const char *path, char *buf, size_t count)
{
    char *fgets_ret = NULL;
    FILE *fd = NULL;
    int rv = 0;

    // opens the file to read the HW_PLATFORM detail of the target
    fd = fopen(path, "r");
    if (fd == NULL)
        return -1;

    // stores the data that is read from the given path into buf
    fgets_ret = fgets(buf, (int)count, fd);
    if (NULL != fgets_ret)
        rv = (int)strlen(buf);
    else
        rv = ferror(fd);

    fclose(fd);
    return rv;
}

/**
 * @brief This function gets the source information from the file.
 *
 * This function receives a buffer variable to store the read information
 * and also receives two different path. The hardware information may be
 * present in any one of the received path. So this function checks in
 * both the paths. This function internally uses read_line_from_file
 * function to read the check and read the hardware details in each path.
 *
 * @param buf hardware details that is read will be stored.
 * @param soc_node_path1 The first path where the file may be present.
 * @param soc_node_path2 The second path where the file may be present.
 * @return Returns the length of buffer.
 */

static int get_soc_info(char *buf, const char *soc_node_path1,
            const char *soc_node_path2)
{
    int ret = 0;

    // checks whether the hw platform detail is present in this path
    ret = read_line_from_file(soc_node_path1, buf, MAX_SOC_INFO_NAME_LEN);
    if (ret < 0) {
        // if the hw platform detail is not present in the former path it checks here
        ret = read_line_from_file(soc_node_path2, buf, MAX_SOC_INFO_NAME_LEN);
        if (ret < 0) {
            ALOGE("getting socinfo(%s, %d) failed.\n", soc_node_path1, ret);
            return ret;
        }
    }
    if (ret && buf[ret - 1] == '\n')
        buf[ret - 1] = '\0';
    return ret;
}

/**
 * @brief finds the cofiguration id value for the particular target.
 *
 * This function reads the soc id detail and hardware platform detail
 * from the target device and generate a generic config file name.
 * If that config file is present then it will be used for configuring
 * that target.
 * If not then based on the target information a config file will be assigned.
 *
 * @param config_file The generic config file name will be stored.
 * @return it returns the config id for the target.
 */

int CNxpNfcConfig::getconfiguration_id(char *config_file)
{
    int config_id = QRD_TYPE_DEFAULT;
    char target_type[MAX_SOC_INFO_NAME_LEN] = {'\0'};
    char soc_info[MAX_SOC_INFO_NAME_LEN] = {'\0'};
    bool nq2xx_present = false;
    char nq_chipid[PROPERTY_VALUE_MAX] = {0};
    char nq_fw_ver[PROPERTY_VALUE_MAX] = {0};
    string strPath;
    int rc = 0;
    int idx = 0;

    rc = get_soc_info(soc_info, SYSFS_SOCID_PATH1, SYSFS_SOCID_PATH2);
    if (rc < 0) {
        ALOGE("get_soc_info(SOC_ID) fail!\n");
        return DEFAULT_CONFIG;
    }
    idx = atoi(soc_info);

    rc = get_soc_info(target_type, SYSFS_HW_PLATFORM_PATH1, SYSFS_HW_PLATFORM_PATH2);
    if (rc < 0) {
        ALOGE("get_soc_info(HW_PLATFORM) fail!\n");
        return DEFAULT_CONFIG;
    }
    rc = __system_property_get("sys.nfc.nq.chipid", nq_chipid);
    if (rc <= 0)
        ALOGE("get sys.nfc.nq.chipid fail, rc = %d\n", rc);
    else
        ALOGD("sys.nfc.nq.chipid = %s\n", nq_chipid);

    nq2xx_present = (!strncmp(nq_chipid, NQ220, PROPERTY_VALUE_MAX))
                 || (!strncmp(nq_chipid, NQ210, PROPERTY_VALUE_MAX));

    rc = __system_property_get("sys.nfc.nq.fwver", nq_fw_ver);
    if (rc <= 0)
        ALOGE("get sys.nfc.nq.fwver fail, rc = %d\n", rc);
    else
        ALOGD("sys.nfc.nq.fwver = %s\n", nq_fw_ver);

    // Converting the HW_PLATFORM detail that is read from target to lowercase
    for (int i=0;target_type[i];i++)
        target_type[i] = tolower(target_type[i]);

    // generating a generic config file name based on the target details
    snprintf(config_file, MAX_DATA_CONFIG_PATH_LEN, "libnfc-%s_%s.conf",
        soc_info, target_type);

    findConfigFilePathFromTransportConfigPaths(config_file, strPath);
    if (file_exist(strPath.c_str()))
        idx = 0;

    if (DEBUG)
        ALOGI("id:%d, config_file_name:%s\n", idx, config_file);

    // if target is QRD platform then config id is assigned here
    if (0 == strncmp(target_type, QRD_HW_PLATFORM, MAX_SOC_INFO_NAME_LEN)) {
        switch (idx)
        {
        case TARGET_GENERIC:
            config_id = CONFIG_GENERIC;
            break;
        case TARGET_MSM8952:
            config_id = QRD_TYPE_DEFAULT;
            strlcpy(config_file, config_name_qrd, MAX_DATA_CONFIG_PATH_LEN);
            break;
        case TARGET_MSM8953:
        case TARGET_MSM8937:
        case TARGET_MSM8940:
        case TARGET_MSM8917:
        case TARGET_MSM8909:
        case TARGET_SDM450:
        case TARGET_SDM632:
        case TARGET_SDM439:
        case TARGET_SDM429:
            if (nq2xx_present) {
                // NQ210 or NQ220
                config_id = QRD_TYPE_DEFAULT;
                strlcpy(config_file, config_name_qrd, MAX_DATA_CONFIG_PATH_LEN);
            } else if(!strncmp(nq_fw_ver, FW_MAJOR_NUM_NQ4xx, FW_MAJOR_NUM_LENGTH)){
                config_id = QRD_TYPE_NQ4XX;
                strlcpy(config_file, config_name_qrd_NQ4XX, MAX_DATA_CONFIG_PATH_LEN);
            } else {
                config_id = QRD_TYPE_NQ3XX;
                strlcpy(config_file, config_name_qrd_NQ3XX, MAX_DATA_CONFIG_PATH_LEN);
            }
            break;
        case TARGET_MSM8976:
        case TARGET_MSM8996:
            config_id = QRD_TYPE_1;
            strlcpy(config_file, config_name_qrd1, MAX_DATA_CONFIG_PATH_LEN);
            break;
        case TARGET_SDM845:
            if (!strncmp(nq_fw_ver, FW_MAJOR_NUM_NQ4xx, FW_MAJOR_NUM_LENGTH)) {
                config_id = QRD_TYPE_NQ4XX;
                strlcpy(config_file, config_name_qrd_NQ4XX, MAX_DATA_CONFIG_PATH_LEN);
            }
            else {
                config_id = QRD_TYPE_NQ3XX;
                strlcpy(config_file, config_name_qrd_NQ3XX, MAX_DATA_CONFIG_PATH_LEN);
            }
            break;
        case TARGET_MSM8998:
        case TARGET_MSM8997:
        case TARGET_SDM660:
        case TARGET_SDM630:
            if (nq2xx_present) {
                // NQ210 or NQ220
                config_id = QRD_TYPE_2;
                strlcpy(config_file, config_name_qrd2, MAX_DATA_CONFIG_PATH_LEN);
            } else {
                config_id = QRD_TYPE_NQ3XX;
                strlcpy(config_file, config_name_qrd_NQ3XX, MAX_DATA_CONFIG_PATH_LEN);
            }
            break;
        default:
            config_id = QRD_TYPE_DEFAULT;
            strlcpy(config_file, config_name_qrd, MAX_DATA_CONFIG_PATH_LEN);
            break;
        }
    }
    // if target is not QRD platform then default config id is assigned here
    else {
        switch (idx)
        {
        case TARGET_GENERIC:
            config_id = CONFIG_GENERIC;
            break;
        case TARGET_MSM8953:
        case TARGET_MSM8937:
        case TARGET_MSM8940:
        case TARGET_MSM8917:
        case TARGET_MSM8909:
        case TARGET_SDM450:
        case TARGET_SDM632:
        case TARGET_SDM439:
        case TARGET_SDM429:
            if (nq2xx_present) {
                // NQ210 or NQ220
                config_id = MTP_TYPE_DEFAULT;
                strlcpy(config_file, config_name_mtp, MAX_DATA_CONFIG_PATH_LEN);
            } else if (!strncmp(nq_fw_ver, FW_MAJOR_NUM_NQ4xx, FW_MAJOR_NUM_LENGTH)){
                config_id = MTP_TYPE_NQ4XX;
                strlcpy(config_file, config_name_mtp_NQ4XX, MAX_DATA_CONFIG_PATH_LEN);
            } else {
                config_id = MTP_TYPE_NQ3XX;
                strlcpy(config_file, config_name_mtp_NQ3XX, MAX_DATA_CONFIG_PATH_LEN);
            }
            break;
        case TARGET_SDM845:
            if (!strncmp(nq_fw_ver, FW_MAJOR_NUM_NQ4xx, FW_MAJOR_NUM_LENGTH)) {
                config_id = MTP_TYPE_NQ4XX;
                strlcpy(config_file, config_name_mtp_NQ4XX, MAX_DATA_CONFIG_PATH_LEN);
            }
            else {
                config_id = MTP_TYPE_NQ3XX;
                strlcpy(config_file, config_name_mtp_NQ3XX, MAX_DATA_CONFIG_PATH_LEN);
            }
            break;
        case TARGET_MSM8998:
        case TARGET_MSM8997:
        case TARGET_SDM660:
        case TARGET_SDM630:
            if (nq2xx_present) {
                // NQ210 or NQ220
                config_id = MTP_TYPE_1;
                strlcpy(config_file, config_name_mtp1, MAX_DATA_CONFIG_PATH_LEN);
            } else {
                config_id = MTP_TYPE_NQ3XX;
                strlcpy(config_file, config_name_mtp_NQ3XX, MAX_DATA_CONFIG_PATH_LEN);
            }
            break;
        default:
            config_id = MTP_TYPE_DEFAULT;
            strlcpy(config_file, config_name_mtp, MAX_DATA_CONFIG_PATH_LEN);
            break;
        }
    }
    if (DEBUG)
        ALOGI("platform config id:%d, config_file_name:%s\n", config_id, config_file);

    return config_id;
}

/*******************************************************************************
**
** Function:    isPrintable()
**
** Description: determine if 'c' is printable
**
** Returns:     1, if printable, otherwise 0
**
*******************************************************************************/
inline bool isPrintable(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
         (c >= '0' && c <= '9') || c == '/' || c == '_' || c == '-' || c == '.';
}

/*******************************************************************************
**
** Function:    isDigit()
**
** Description: determine if 'c' is numeral digit
**
** Returns:     true, if numerical digit
**
*******************************************************************************/
inline bool isDigit(char c, int base) {
  if ('0' <= c && c <= '9') return true;
  if (base == 16) {
    if (('A' <= c && c <= 'F') || ('a' <= c && c <= 'f')) return true;
  }
  return false;
}

/*******************************************************************************
**
** Function:    getDigitValue()
**
** Description: return numerical value of a decimal or hex char
**
** Returns:     numerical value if decimal or hex char, otherwise 0
**
*******************************************************************************/
inline int getDigitValue(char c, int base) {
  if ('0' <= c && c <= '9') return c - '0';
  if (base == 16) {
    if ('A' <= c && c <= 'F')
      return c - 'A' + 10;
    else if ('a' <= c && c <= 'f')
      return c - 'a' + 10;
  }
  return 0;
}

/*******************************************************************************
**
** Function:    findConfigFilePathFromTransportConfigPaths()
**
** Description: find a config file path with a given config name from transport
**              config paths
**
** Returns:     none
**
*******************************************************************************/
void findConfigFilePathFromTransportConfigPaths(const string& configName,
                                                string& filePath) {
  for (int i = 0; i < transport_config_path_size - 1; i++) {
    filePath.assign(transport_config_paths[i]);
    filePath += configName;
    struct stat file_stat;
    if (stat(filePath.c_str(), &file_stat) == 0 && S_ISREG(file_stat.st_mode)) {
      return;
    }
  }
  filePath.assign(transport_config_paths[transport_config_path_size - 1]);
  filePath += configName;
}

/*******************************************************************************
**
** Function:    CNxpNfcConfig::readConfig()
**
** Description: read Config settings and parse them into a linked list
**              move the element from linked list to a array at the end
**
** Returns:     1, if there are any config data, 0 otherwise
**
*******************************************************************************/
bool CNxpNfcConfig::readConfig(const char* name, bool bResetContent) {
  enum {
    BEGIN_LINE = 1,
    TOKEN,
    STR_VALUE,
    NUM_VALUE,
    BEGIN_HEX,
    BEGIN_QUOTE,
    END_LINE
  };

  uint8_t* p_config = nullptr;
  size_t config_size = readConfigFile(name, &p_config);
  if (p_config == nullptr) {
    ALOGE("%s Cannot open config file %s\n", __func__, name);
    if (bResetContent) {
      ALOGE("%s Using default value for all settings\n", __func__);
      mValidFile = false;
    }
    return false;
  }

  string token;
  string strValue;
  unsigned long numValue = 0;
  CNxpNfcParam* pParam = NULL;
  int i = 0;
  int base = 0;
  char c;
  int bflag = 0;
  state = BEGIN_LINE;
  mCurrentFile = name;
  struct stat buf;

  if (strcmp(default_nxp_config_path, name) == 0) {
    config_crc32_ = sparse_crc32(0, p_config, config_size);
  }
  if (strcmp(nxp_rf_config_path, name) == 0) {
    config_crc32_rf_ = sparse_crc32(0, p_config, config_size);
  }
  if (strcmp(transit_config_path, name) == 0) {
    config_crc32_tr_ = sparse_crc32(0, p_config, config_size);
  }
    stat (name, &buf);

    if(mDynamConfig)
        m_timeStamp = (unsigned long)buf.st_mtime;
    else
    {
        if(strcmp(default_nxp_config_path, name) == 0)
            m_timeStamp = (unsigned long)buf.st_mtime;
    }

  mValidFile = true;
  if (size() > 0) {
    if (bResetContent)
      clean();
    else
      moveToList();
  }

  for (size_t offset = 0; offset != config_size; ++offset) {
    c = p_config[offset];
    switch (state & 0xff) {
      case BEGIN_LINE:
        if (c == '#')
          state = END_LINE;
        else if (isPrintable(c)) {
          i = 0;
          token.erase();
          strValue.erase();
          state = TOKEN;
          token.push_back(c);
        }
        break;
      case TOKEN:
        if (c == '=') {
          token.push_back('\0');
          state = BEGIN_QUOTE;
        } else if (isPrintable(c))
          token.push_back(c);
        else
          state = END_LINE;
        break;
      case BEGIN_QUOTE:
        if (c == '"') {
          state = STR_VALUE;
          base = 0;
        } else if (c == '0')
          state = BEGIN_HEX;
        else if (isDigit(c, 10)) {
          state = NUM_VALUE;
          base = 10;
          numValue = getDigitValue(c, base);
          i = 0;
        } else if (c == '{') {
          state = NUM_VALUE;
          bflag = 1;
          base = 16;
          i = 0;
          Set(IsStringValue);
        } else
          state = END_LINE;
        break;
      case BEGIN_HEX:
        if (c == 'x' || c == 'X') {
          state = NUM_VALUE;
          base = 16;
          numValue = 0;
          i = 0;
          break;
        } else if (isDigit(c, 10)) {
          state = NUM_VALUE;
          base = 10;
          numValue = getDigitValue(c, base);
          break;
        } else if (c != '\n' && c != '\r') {
          state = END_LINE;
          break;
        }
      // fall through to numValue to handle numValue

      case NUM_VALUE:
        if (isDigit(c, base)) {
          numValue *= base;
          numValue += getDigitValue(c, base);
          ++i;
        } else if (bflag == 1 &&
                   (c == ' ' || c == '\r' || c == '\n' || c == '\t')) {
          break;
        } else if (base == 16 &&
                   (c == ',' || c == ':' || c == '-' || c == ' ' || c == '}')) {
          if (c == '}') {
            bflag = 0;
          }
          if (i > 0) {
            int n = (i + 1) / 2;
            while (n-- > 0) {
              numValue = numValue >> (n * 8);
              unsigned char c = (numValue)&0xFF;
              strValue.push_back(c);
            }
          }

          Set(IsStringValue);
          numValue = 0;
          i = 0;
        } else {
          if (c == '\n' || c == '\r') {
            if (bflag == 0) {
              state = BEGIN_LINE;
            }
          } else {
            if (bflag == 0) {
              state = END_LINE;
            }
          }
          if (Is(IsStringValue) && base == 16 && i > 0) {
            int n = (i + 1) / 2;
            while (n-- > 0) strValue.push_back(((numValue >> (n * 8)) & 0xFF));
          }
          if (strValue.length() > 0)
            pParam = new CNxpNfcParam(token.c_str(), strValue);
          else
            pParam = new CNxpNfcParam(token.c_str(), numValue);
          add(pParam);
          strValue.erase();
          numValue = 0;
        }
        break;
      case STR_VALUE:
        if (c == '"') {
          strValue.push_back('\0');
          state = END_LINE;
          pParam = new CNxpNfcParam(token.c_str(), strValue);
          add(pParam);
        } else if (isPrintable(c))
          strValue.push_back(c);
        break;
      case END_LINE:
        if (c == '\n' || c == '\r') state = BEGIN_LINE;
        break;
      default:
        break;
    }
  }

  delete[] p_config;

  moveFromList();
  return size() > 0;
}

/*******************************************************************************
**
** Function:    CNxpNfcConfig::CNxpNfcConfig()
**
** Description: class constructor
**
** Returns:     none
**
*******************************************************************************/
CNxpNfcConfig::CNxpNfcConfig () :
    mValidFile (true),
    mDynamConfig(true),
    m_timeStamp (0),
    state (0)
{
}

/*******************************************************************************
**
** Function:    CNxpNfcConfig::~CNxpNfcConfig()
**
** Description: class destructor
**
** Returns:     none
**
*******************************************************************************/
CNxpNfcConfig::~CNxpNfcConfig ()
{
}

/**
 * @brief checks whether the given file exist.
 *
 * This function gets the file name and checks whether the given file
 * exist in the particular path.Internaly it uses stat system call to
 * find the existance.
 *
 * @param filename The name of the file whose existance has to be checked.
 * @return it returns true if the given file name exist.
 */
int CNxpNfcConfig::file_exist (const char* filename)
{
    struct stat   buffer;
    return (stat (filename, &buffer) == 0);
}

/*******************************************************************************
**
** Function:    CNxpNfcConfig::GetInstance()
**
** Description: get class singleton object
**
** Returns:     none
**
*******************************************************************************/
CNxpNfcConfig& CNxpNfcConfig::GetInstance ()
{
    static CNxpNfcConfig theInstance;
    int gconfigpathid = 0;
    char config_name_generic[MAX_DATA_CONFIG_PATH_LEN] = {'\0'};

    if (theInstance.size () == 0 && theInstance.mValidFile)
    {
        string strPath;
        if (alternative_config_path [0] != '\0')
        {
            strPath.assign (alternative_config_path);
            strPath += config_name;
            theInstance.readConfig (strPath.c_str (), true);
            if (!theInstance.empty ())
            {
                return theInstance;
            }
        }
        findConfigFilePathFromTransportConfigPaths(config_name, strPath);
        //checks whether the default config file is present in th target
        if (theInstance.file_exist(strPath.c_str()))
        {
            ALOGI("default config file exists = %s, dynamic selection disabled", strPath.c_str());
            theInstance.mDynamConfig = false;
            theInstance.readConfig(strPath.c_str(), true);
            /*
             * if libnfc-nxp.conf exists then dynamic selection will
             * be turned off by default we will not have this file.
             */
            return theInstance;
        }

        gconfigpathid = theInstance.getconfiguration_id(config_name_generic);
        findConfigFilePathFromTransportConfigPaths(config_name_generic, strPath);
        if (!(theInstance.file_exist(strPath.c_str()))) {
           ALOGI("no matching file found, using default file for stability\n");
           findConfigFilePathFromTransportConfigPaths(config_name_default, strPath);
        }
        ALOGI("config file used = %s\n", strPath.c_str());
        theInstance.readConfig (strPath.c_str (), true);
#if(NXP_EXTNS == TRUE)
        theInstance.readNxpTransitConfig("nxpTransit");
#endif
    }

    return theInstance;
}

/*******************************************************************************
**
** Function:    CNxpNfcConfig::getValue()
**
** Description: get a string value of a setting
**
** Returns:     true if setting exists
**              false if setting does not exist
**
*******************************************************************************/
bool CNxpNfcConfig::getValue(const char* name, char* pValue, size_t len) const {
  const CNxpNfcParam* pParam = find(name);
  if (pParam == NULL) return false;

  if (pParam->str_len() > 0) {
    memset(pValue, 0, len);
    memcpy(pValue, pParam->str_value(), pParam->str_len());
    return true;
  }
  return false;
}

bool CNxpNfcConfig::getValue(const char* name, char* pValue, long len,
                          long* readlen) const {
  const CNxpNfcParam* pParam = find(name);
  if (pParam == NULL) return false;

  if (pParam->str_len() > 0) {
    if (pParam->str_len() <= (unsigned long)len) {
      memset(pValue, 0, len);
      memcpy(pValue, pParam->str_value(), pParam->str_len());
      *readlen = pParam->str_len();
    } else {
      *readlen = -1;
    }

    return true;
  }
  return false;
}

/*******************************************************************************
**
** Function:    CNxpNfcConfig::getValue()
**
** Description: get a long numerical value of a setting
**
** Returns:     true if setting exists
**              false if setting does not exist
**
*******************************************************************************/
bool CNxpNfcConfig::getValue(const char* name, unsigned long& rValue) const {
  const CNxpNfcParam* pParam = find(name);
  if (pParam == NULL) return false;

  if (pParam->str_len() == 0) {
    rValue = static_cast<unsigned long>(pParam->numValue());
    return true;
  }
  return false;
}

/*******************************************************************************
**
** Function:    CNxpNfcConfig::getValue()
**
** Description: get a short numerical value of a setting
**
** Returns:     true if setting exists
**              false if setting does not exist
**
*******************************************************************************/
bool CNxpNfcConfig::getValue(const char* name, unsigned short& rValue) const {
  const CNxpNfcParam* pParam = find(name);
  if (pParam == NULL) return false;

  if (pParam->str_len() == 0) {
    rValue = static_cast<unsigned short>(pParam->numValue());
    return true;
  }
  return false;
}

/*******************************************************************************
**
** Function:    CNxpNfcConfig::find()
**
** Description: search if a setting exist in the setting array
**
** Returns:     pointer to the setting object
**
*******************************************************************************/
const CNxpNfcParam* CNxpNfcConfig::find(const char* p_name) const {
  if (size() == 0) return NULL;

  for (const_iterator it = begin(), itEnd = end(); it != itEnd; ++it) {
    if (**it < p_name) {
      continue;
    } else if (**it == p_name) {
      if ((*it)->str_len() > 0) {
        NXPLOG_EXTNS_D("%s found %s=%s\n", __func__, p_name,
                       (*it)->str_value());
      } else {
        NXPLOG_EXTNS_D("%s found %s=(0x%lx)\n", __func__, p_name,
                       (*it)->numValue());
      }
      return *it;
    } else
      break;
  }
  return NULL;
}

/*******************************************************************************
**
** Function:    CNxpNfcConfig::readNxpTransitConfig()
**
** Description: read Config settings from transit conf file
**
** Returns:     none
**
*******************************************************************************/
void CNxpNfcConfig::readNxpTransitConfig(const char* fileName) const {
  ALOGD("readNxpTransitConfig-Enter..Reading %s", fileName);
  CNxpNfcConfig::GetInstance().readConfig(fileName, false);
}

/*******************************************************************************
**
** Function:    CNxpNfcConfig::readNxpRFConfig()
**
** Description: read Config settings from RF conf file
**
** Returns:     none
**
*******************************************************************************/
void CNxpNfcConfig::readNxpRFConfig(const char* fileName) const {
  ALOGD("readNxpRFConfig-Enter..Reading %s", fileName);
  CNxpNfcConfig::GetInstance().readConfig(fileName, false);
}

/*******************************************************************************
**
** Function:    CNfcConfig::clean()
**
** Description: reset the setting array
**
** Returns:     none
**
*******************************************************************************/
void CNxpNfcConfig::clean() {
  if (size() == 0) return;

  for (iterator it = begin(), itEnd = end(); it != itEnd; ++it) delete *it;
  clear();
}

/*******************************************************************************
**
** Function:    CNxpNfcConfig::Add()
**
** Description: add a setting object to the list
**
** Returns:     none
**
*******************************************************************************/
void CNxpNfcConfig::add(const CNxpNfcParam* pParam) {
  if (m_list.size() == 0) {
    m_list.push_back(pParam);
    return;
  }
  if ((mCurrentFile.find("nxpTransit") != std::string::npos) &&
      !isAllowed(pParam->c_str())) {
    ALOGD("%s Token restricted. Returning", __func__);
    return;
  }
  for (list<const CNxpNfcParam*>::iterator it = m_list.begin(),
                                        itEnd = m_list.end();
       it != itEnd; ++it) {
    if (**it < pParam->c_str()) continue;
    if (**it == pParam->c_str())
      m_list.insert(m_list.erase(it), pParam);
    else
      m_list.insert(it, pParam);

    return;
  }
  m_list.push_back(pParam);
}

/*******************************************************************************
**
** Function:    CNxpNfcConfig::dump()
**
** Description: prints all elements in the list
**
** Returns:     none
**
*******************************************************************************/
void CNxpNfcConfig::dump() {
  ALOGD("%s Enter", __func__);

  for (list<const CNxpNfcParam*>::iterator it = m_list.begin(),
                                        itEnd = m_list.end();
       it != itEnd; ++it) {
    if ((*it)->str_len() > 0)
      ALOGD("%s %s \t= %s", __func__, (*it)->c_str(), (*it)->str_value());
    else
      ALOGD("%s %s \t= (0x%0lX)\n", __func__, (*it)->c_str(),
            (*it)->numValue());
  }
}
/*******************************************************************************
**
** Function:    CNfcConfig::isAllowed()
**
** Description: checks if token update is allowed
**
** Returns:     true if allowed else false
**
*******************************************************************************/
bool CNxpNfcConfig::isAllowed(const char* name) {
  string token(name);
  bool stat = false;
  if ((token.find("P2P_LISTEN_TECH_MASK") != std::string::npos) ||
      (token.find("HOST_LISTEN_TECH_MASK") != std::string::npos) ||
      (token.find("UICC_LISTEN_TECH_MASK") != std::string::npos) ||
      (token.find("NXP_ESE_LISTEN_TECH_MASK") != std::string::npos) ||
      (token.find("POLLING_TECH_MASK") != std::string::npos) ||
      (token.find("NXP_RF_CONF_BLK") != std::string::npos) ||
      (token.find("NXP_CN_TRANSIT_BLK_NUM_CHECK_ENABLE") !=
       std::string::npos) ||
      (token.find("NXP_FWD_FUNCTIONALITY_ENABLE") != std::string::npos))

  {
    stat = true;
  }
  return stat;
}
/*******************************************************************************
**
** Function:    CNxpNfcConfig::moveFromList()
**
** Description: move the setting object from list to array
**
** Returns:     none
**
*******************************************************************************/
void CNxpNfcConfig::moveFromList() {
  if (m_list.size() == 0) return;

  for (list<const CNxpNfcParam*>::iterator it = m_list.begin(),
                                        itEnd = m_list.end();
       it != itEnd; ++it)
    push_back(*it);
  m_list.clear();
}

/*******************************************************************************
**
** Function:    CNxpNfcConfig::moveToList()
**
** Description: move the setting object from array to list
**
** Returns:     none
**
*******************************************************************************/
void CNxpNfcConfig::moveToList() {
  if (m_list.size() != 0) m_list.clear();

  for (iterator it = begin(), itEnd = end(); it != itEnd; ++it)
    m_list.push_back(*it);
  clear();
}

bool CNxpNfcConfig::isModified() {
  FILE* fd = fopen(config_timestamp_path, "r+");
  if (fd == nullptr) {
    ALOGE("%s Unable to open file '%s' - assuming modified", __func__,
          config_timestamp_path);
    return true;
  }

  uint32_t stored_crc32 = 0;
  fread(&stored_crc32, sizeof(uint32_t), 1, fd);
  fclose(fd);
 
  return stored_crc32 != config_crc32_;
}

bool CNxpNfcConfig::isModified(const char* pName) {
  FILE* fd = nullptr;
  bool isRfFile = false;
  if (strcmp(nxp_rf_config_path, pName) == 0) {
    isRfFile = true;
    fd = fopen(rf_config_timestamp_path, "r+");
  }
  else if (strcmp(transit_config_path, pName) == 0) {
    fd = fopen(tr_config_timestamp_path, "r+");
  }
  if (fd == nullptr) {
    ALOGE("%s Unable to open file '%s' - assuming modified", __func__,
          (isRfFile?rf_config_timestamp_path:tr_config_timestamp_path));
    return true;
  }

  uint32_t stored_crc32 = 0;
  fread(&stored_crc32, sizeof(uint32_t), 1, fd);
  fclose(fd);

  if(isRfFile)
    return stored_crc32 != config_crc32_rf_;
  else
    return stored_crc32 != config_crc32_tr_;
}

void CNxpNfcConfig::resetModified() {
  FILE* fd = nullptr;

  fd = fopen(config_timestamp_path, "w+");
  if (fd == nullptr) {
    ALOGE("%s Unable to open file '%s' for writing", __func__,
        config_timestamp_path);
  } else {
    fwrite(&config_crc32_, sizeof(uint32_t), 1, fd);
    fclose(fd);
  }
  fd = fopen(rf_config_timestamp_path, "w+");
  if (fd == nullptr) {
    ALOGE("%s Unable to open file '%s' for writing", __func__,
        rf_config_timestamp_path);
  } else {
    fwrite(&config_crc32_rf_, sizeof(uint32_t), 1, fd);
    fclose(fd);
  }

  fd = fopen(tr_config_timestamp_path, "w+");
  if (fd == nullptr) {
    ALOGE("%s Unable to open file '%s' for writing", __func__,
        tr_config_timestamp_path);
  } else {
    fwrite(&config_crc32_tr_, sizeof(uint32_t), 1, fd);
    fclose(fd);
  }
}

/*******************************************************************************
**
** Function:    CNxpNfcParam::CNxpNfcParam()
**
** Description: class constructor
**
** Returns:     none
**
*******************************************************************************/
CNxpNfcParam::CNxpNfcParam() : m_numValue(0) {}

/*******************************************************************************
**
** Function:    CNxpNfcParam::~CNxpNfcParam()
**
** Description: class destructor
**
** Returns:     none
**
*******************************************************************************/
CNxpNfcParam::~CNxpNfcParam() {}

/*******************************************************************************
**
** Function:    CNxpNfcParam::CNxpNfcParam()
**
** Description: class copy constructor
**
** Returns:     none
**
*******************************************************************************/
CNxpNfcParam::CNxpNfcParam(const char* name, const string& value)
    : string(name), m_str_value(value), m_numValue(0) {}

/*******************************************************************************
**
** Function:    CNxpNfcParam::CNxpNfcParam()
**
** Description: class copy constructor
**
** Returns:     none
**
*******************************************************************************/
CNxpNfcParam::CNxpNfcParam(const char* name, unsigned long value)
    : string(name), m_numValue(value) {}

/*******************************************************************************
**
** Function:    readOptionalConfig()
**
** Description: read Config settings from an optional conf file
**
** Returns:     none
**
*******************************************************************************/
void readOptionalConfig(const char* extra) {
  string strPath;
  string configName(extra_config_base);
  configName += extra;
  configName += extra_config_ext;

  if (alternative_config_path[0] != '\0') {
    strPath.assign(alternative_config_path);
    strPath += configName;
  } else {
    findConfigFilePathFromTransportConfigPaths(configName, strPath);
  }

  CNxpNfcConfig::GetInstance().readConfig(strPath.c_str(), false);
}

/*******************************************************************************
**
** Function:    GetStrValue
**
** Description: API function for getting a string value of a setting
**
** Returns:     True if found, otherwise False.
**
*******************************************************************************/
extern "C" int GetNxpStrValue (const char* name, char* pValue, unsigned long len)
{
    CNxpNfcConfig& rConfig = CNxpNfcConfig::GetInstance ();

    return rConfig.getValue (name, pValue, len);
}

/*******************************************************************************
**
** Function:    GetByteArrayValue()
**
** Description: Read byte array value from the config file.
**
** Parameters:
**              name    - name of the config param to read.
**              pValue  - pointer to input buffer.
**              bufflen - input buffer length.
**              len     - out parameter to return the number of bytes read from config file,
**                        return -1 in case bufflen is not enough.
**
** Returns:     true[1] if config param name is found in the config file, else false[0]
**
*******************************************************************************/
extern "C" int GetNxpByteArrayValue (const char* name, char* pValue, long bufflen, long *len)
{
    CNxpNfcConfig& rConfig = CNxpNfcConfig::GetInstance ();

    return rConfig.getValue (name, pValue, bufflen,len);
}

/*******************************************************************************
**
** Function:    GetNumValue
**
** Description: API function for getting a numerical value of a setting
**
** Returns:     true, if successful
**
*******************************************************************************/
extern "C" int GetNxpNumValue (const char* name, void* pValue, unsigned long len)
{
    if (!pValue)
        return false;

    CNxpNfcConfig& rConfig = CNxpNfcConfig::GetInstance ();
    const CNxpNfcParam* pParam = rConfig.find (name);

    if (pParam == NULL)
        return false;

    unsigned long v = pParam->numValue ();
    if (v == 0 && pParam->str_len () > 0 && pParam->str_len () < 4)
    {
        const unsigned char* p = (const unsigned char*) pParam->str_value ();
        for (unsigned int i = 0 ; i < pParam->str_len (); ++i)
        {
            v *= 256;
            v += *p++;
        }
    }
    switch (len)
    {
#if(NFC_ARCH_TYPE == 64)
    case sizeof(unsigned long):
        *(static_cast<unsigned long*>(pValue)) = (unsigned long) v;
        break;
#endif
    case sizeof(unsigned int):
        *(static_cast<unsigned int*>(pValue)) = (unsigned int) v;
        break;
    case sizeof(unsigned short):
        *(static_cast<unsigned short*>(pValue)) = (unsigned short) v;
        break;
    case sizeof(unsigned char):
        *(static_cast<unsigned char*> (pValue)) = (unsigned char) v;
        break;
    default:
        return false;
    }
    return true;
}

/*******************************************************************************
**
** Function:    resetConfig
**
** Description: reset settings array
**
** Returns:     none
**
*******************************************************************************/
extern "C" void resetNxpConfig ()
{
    CNxpNfcConfig& rConfig = CNxpNfcConfig::GetInstance ();

    rConfig.clean ();
}

/*******************************************************************************
**
** Function:    isNxpConfigModified()
**
** Description: check if config file has modified
**
** Returns:     0 if not modified, 1 otherwise.
**
*******************************************************************************/
extern "C" int isNxpConfigModified ()
{
    CNxpNfcConfig& rConfig = CNxpNfcConfig::GetInstance ();
    return rConfig.isModified();
}

/*******************************************************************************
**
** Function:    isNxpRFConfigModified()
**
** Description: check if config file has modified
**
** Returns:     0 if not modified, 1 otherwise.
**
*******************************************************************************/
extern "C" int isNxpRFConfigModified() {
  int retRF = 0, rettransit = 0, ret = 0;
  CNxpNfcConfig& rConfig = CNxpNfcConfig::GetInstance();
  retRF = rConfig.isModified(nxp_rf_config_path);
  rettransit = rConfig.isModified(transit_config_path);
  ret = retRF | rettransit;
  ALOGD("ret RF or Transit value %d", ret);
  return ret;
}

/*******************************************************************************
**
** Function:    updateNxpConfigTimestamp()
**
** Description: update if config file has modified
**
** Returns:     0 if not modified, 1 otherwise.
**
*******************************************************************************/
extern "C" int updateNxpConfigTimestamp()
{
    CNxpNfcConfig& rConfig = CNxpNfcConfig::GetInstance();
    rConfig.resetModified();
    return 0;
}
