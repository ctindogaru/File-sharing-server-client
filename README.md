# File-sharing-server-client  

A server-client application which allows you to transfer files between users.  

How to use it:  
1. Type "make"  
2. Start the server: ./selectserver <port_server> <users_config_file> <shared_files>  
port_server -> you need to choose a port for the server  
users_config_file -> a file with the user credentials  
shared_files -> a file with each user's shareable files  
Example: ./selectserver 1000 users_config shared_files  
3. Start the clients: ./client <IP_server> <port_server>  
Example: ./client 127.0.0.1 1000  
  
Commands available on the client:  
1. login <user_name> <pass_word>  
Example: login Doru protocoale  
2. logout  
3. getuserlist -> you can see all the users from the system  
4. getfilelist <user_name> -> you can see the user available files  
5. upload <file_name> -> you can upload a file to server  
Example: upload test_file.txt  
If you're logged in as Doru, this command will upload test_file.txt in Doru's directory.  
6. download <user_name> <file_name>  
Example: download Elena Catalog.xlsx  
If you're logged in as Doru, this command will download Catalog.xlsx from Elena's directory in Doru's directory  
7. share <file_name>  
Makes the file shareable. This means all users can download it.  
8. unshare <file_name>  
Makes the file private. This means no one can download the file.  
  
All the activity from as session will be logged in a file called log.txt.  
  
ENJOY!  
