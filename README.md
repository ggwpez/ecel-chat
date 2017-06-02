Clone &Compile& Start:

git clone https://github.com/ggwpez/ecel-chat && cd ecel-chat && git submodule init && git submodule update && mkdir ecel-chat-bin && cd ecel-chat-bin && qmake ../chat.pro && make && cd .. && mkdir ecel-bin && cd ecel-bin && cmake ../ecel/CMakeLists.txt -B. && make && cp ecel ../ecel-chat-bin/ecel && cd ../ecel-chat-bin && ./chat

Start with config:
./chat /load,sample.config.txt
