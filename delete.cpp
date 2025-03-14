 // 進入主迴圈
    while (true)
    {
        // 建立讀/寫檔案描述子集合
        fd_set readSet, writeSet;
        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);

        // 用來追蹤最大的 fd
        int maxFd = 0;

        // 1) 加入所有 server socket 到 readSet
        for (size_t i = 0; i < serverSockets.size(); ++i)
        {
            int sfd = serverSockets[i];
            FD_SET(sfd, &readSet);
            if (sfd > maxFd) maxFd = sfd;
        }

        // 2) 加入所有 client fd 到 readSet / writeSet
        for (std::map<int, ClientConnection>::iterator it = clients.begin();
             it != clients.end(); ++it)
        {
            int cfd = it->first;
            ClientConnection &conn = it->second;

            // 如果該連線需要讀，就加進 readSet
            if (conn.needsRead()) {
                FD_SET(cfd, &readSet);
                if (cfd > maxFd) maxFd = cfd;
            }
            // 如果該連線需要寫，就加進 writeSet
            if (conn.needsWrite()) {
                FD_SET(cfd, &writeSet);
                if (cfd > maxFd) maxFd = cfd;
            }
        }

        // 3) 呼叫 select()
        int readyCount = select(maxFd + 1, &readSet, &writeSet, NULL, NULL);
        if (readyCount < 0)
        {
            if (errno == EINTR) {
                // 被訊號打斷就繼續
                continue;
            }
            std::cerr << "select() error: " << strerror(errno) << std::endl;
            break; // 出現嚴重錯誤時，可以選擇跳出或其它處理
        }

        // 4) 檢查各個 server socket 是否有新連線
        for (size_t i = 0; i < serverSockets.size(); ++i)
        {
            int sfd = serverSockets[i];
            // 若 server socket 在 readSet 中，表示可以 accept 新連線
            if (FD_ISSET(sfd, &readSet))
            {
                // 可能有多個新連線，在非阻塞模式下要用迴圈 accept
                while (true)
                {
                    sockaddr_in clientAddr;
                    socklen_t addrLen = sizeof(clientAddr);
                    int clientFd = accept(sfd, (sockaddr*)&clientAddr, &addrLen);
                    if (clientFd < 0)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // 表示當前沒有更多新連線
                            break;
                        }
                        else {
                            std::cerr << "accept() error: "
                                      << strerror(errno) << std::endl;
                            break;
                        }
                    }

                    // clientFd 已經繼承了父 socket 的 NONBLOCK 屬性 (在 Linux 下)
                    std::cout << "New client connected: " << clientFd << std::endl;
                    // 建立一個 ClientConnection 放進 map
                    clients.insert(std::make_pair(clientFd, ClientConnection(clientFd)));
                }
            }
        }

        // 5) 檢查所有現有 client FD，是否可讀/可寫
        //    需要在迴圈中動態刪除 map 元素，所以要小心 iterator
        for (std::map<int, ClientConnection>::iterator it = clients.begin();
        it != clients.end(); )
        {
            int cfd = it->first;
            ClientConnection &conn = it->second;

            bool closed = false;

            // (A) 若可讀 => read
            if (FD_ISSET(cfd, &readSet)) {
                int n = conn.readData();
                if (n <= 0) {
                    std::cout << "[Client " << cfd << "] disconnected.\n";
                    close(cfd);
                    std::map<int, ClientConnection>::iterator tmp = it;
                    ++it;
                    clients.erase(tmp);
                    closed = true; 
                }
            }

            // (B) 若可寫 => write
            if (!closed && FD_ISSET(cfd, &writeSet)) {
                int sent = conn.writeData();
                if (sent < 0) {
                    // 檢查 errno, 若不是 EAGAIN / EWOULDBLOCK 就關閉
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        std::cerr << "[Client " << cfd << "] write error: "
                                << strerror(errno) << std::endl;
                        close(cfd);
                        std::map<int, ClientConnection>::iterator tmp = it;
                        ++it;
                        clients.erase(tmp);
                        closed = true;
                    }
                } else {
                    // sent >= 0 => 成功寫出部分或全部
                    // 如果全部送完 (conn.needsWrite() == false)，
                    // 可以視需求斷線，或等待下一次發送(keep-alive)。
                }
            }

            if (!closed) {
                ++it; // 繼續下個客戶端
            }
        }

        //Timeout 
    }
