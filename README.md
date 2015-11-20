# fork and shm HW

#### 請使用以下方法在 32-bit Unix 環境編譯：

* gcc pA.c -o pA -lrt
* gcc pB.c -o pB -lrt

#### 目前問題回報

* 在 pB 如果沒有每行 printf 東西的話會不能跑，非常神祕

圖片支援：http://imgur.com/xD1pxUJ

* pB 在 -1, -2 時有 bug (match, strres)，修理中！


##### There are lots of bugs Q_Q
