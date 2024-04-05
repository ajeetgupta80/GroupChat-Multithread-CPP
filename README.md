# GroupChat-Multithread-CPP

GroupChat is a client-server multithreaded unix based chat application entirely written in c++ that runs on cli.
In this different clients can connect to same server over different threads and communicate among themselves. 
mutex locks are used for safety purpose so during the client thread accessing critial-section it should be safe synchronization.

![](/img.jpeg)

**BUILD PROCESS **

For running the code there are two ways either run through Makefile or compile.sh file.

**Makefile**:
        
           make all
          ./server <port>
          ./client <port>
        
        
**compile.sh**:
        
          ./compile.sh 
          ./server <port>
          ./client <port>
