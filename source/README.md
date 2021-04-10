#Dependencies install....

npm install -g node-gyp
npm install -g node-addon-api

node-gyp rebuild

#Windows:

#for windows need to install windows-build-tools before rebuild

npm install -g windows-build-tools

#Linux:

#for linux need to install gcc before rebuild

sudo apt-get update
sudo apt-get install gcc build-essential