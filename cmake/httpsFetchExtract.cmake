get_filename_component(FFS_ObjStoreDir ${ExternalData_CUSTOM_FILE}DIRECTORY)
get_filename_component(FFS_InputFilename ${ExternalData_CUSTOM_LOCATION}NAME)
get_filename_component(FFS_OutputFilename ${ExternalData_CUSTOM_FILE}NAME)

#execute_process(COMMAND sftp sftp.company.com:/${ExternalData_CUSTOM_LOCATION}RESULT_VARIABLE FFS_SftpResult
#  OUTPUT_QUIETERROR_VARIABLE FFS_SftpErr
#  )

#if(FFS_SftpResult)
#  set(ExternalData_CUSTOM_ERROR "Failed to fetch from SFTP - ${FFS_SftpErr}")
#else(FFS_SftpResult)
#  file(MAKE_DIRECTORY${FFS_ObjStoreDir})
#  file(RENAME${FFS_InputFilename}${FFS_ObjStoreDir}/${FFS_OutputFilename})
#endif(FFS_SftpResult)

file(DOWNLOAD ${ExternalData_CUSTOM_FILE} ${ExternalData_CUSTOM_LOCATION} SHOW_PROGRESS TIMEOUT 120)
