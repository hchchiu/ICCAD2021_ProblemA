# ICCAD2021_ProblemA

## Update Log
- 2021/07/01 23:30
	- [修改]	Topological 順序改為由PI開始往後找
	- [修改]	`piset` 型態改為 `set`
	- [修改]  跑完Topological Sort時即更改Graph裡面的netlist
	- [新增]  `BitWiseOperation()`，計算seed
	- [新增]  `graph2Blif()`，只有先打出架構(可無視)
	- [新增]  `main()` 裡面 `srand(time(NULL))`
	- [新增]  每一個node都有seed
- 2021/07/03 22:00
	- [新增]  `MatchInfo struct`，patch資訊儲存架構
	- [新增]  `structureCompareMain`，structure比較主要架構
	- [新增]  `structureCompareOper`，structure運算核心
	- [新增]  `IsGateTypeEqual`，判斷Gate是否相同
	- [新增]  `IsFaninEqual`，判斷Fanin是否相同
	- [新增]  `IsFaninVisited`，判斷Fanin是否都走過
	- [新增]  `IsVisited`，判斷是否走過
- 2021/07/04 15:00
	- [新增]  `MatchInfo struct`，新增`goldenRemoveNode`
	- [新增]  `structureCompareMain`，修改`goldenRemoveNode`
	- [新增]  `structureCompareOper`，修改`goldenRemoveNode`
- 2021/07/04 15:08
	- [新增]  `graph2Blif`，將傳入的graph轉成blif file
	- [新增]  `netlist2Blif`，找到node的Fanout的node
	- [新增]  `node2Blif`，判斷node的type，輸出對應的結果ex:and 11 1
	- [新增]  `buildMiter`，把Original和Golden的對應的PO接到XOR做Miter
	- [新增]  `struct` `Graph`，加入name紀錄屬於R1 or R2 or G1
	- [新增]  `struct` `Node`，加入name紀錄屬於R1 or R2 or G1
- 2021/07/04 20:50
	- [修改]	`verilog2Graph`，分成2 fanin gate時realgate沒有更改值
	- [修改]	`node2Blif`，PI應該都是一樣不需要加tag，新增assign判斷
- 2021/07/14 02:26
 	- [新增]  `outputConst`，輸出constant的blif(1'b0,1'b1)
 	- [新增]  `outputBlif`，增加faninConst判斷是否有const
	- [修改]	`randomSimulation`，移除set equal判斷(之後需修改)
- 2021/07/18 22:15
 	- [新增]  在`Graph`裡新增`Constants`紀錄`1'b0`、`1'b1`位置
 	- [新增]  在`loadFile`裡resize`Constants`空間
 	- [新增]  在`verilog2graph`裡取得`Constants`位置
- 2021/07/19 01:20
 	- [新增]  `createPatch`，將GoldenRemoveNode當作是patch
 	- [新增]  `compareNetlist`，將GoldenRemoveNode利用BLIF FILE接上Original並判斷是否一樣
 	- [新增]  `outputPatchDotNames`，用來輸出patch的BLIF FILE的.names
 	- [新增]  `toString`，將int轉為string
- 2021/07/20 16:45
 	- [新增]  `generatePatchVerilog`，輸出patch
 	- [新增]  `generateInstruction`，產生patch指令
 	- [新增]  `getTypeString`，取得Gate String
 	- [新增]  `generateDeclare`，輸出變數宣告
- 2021/07/22 16:33
	- [修改]  `generatePatchVerilog`，修副宣告bus前要加`\`
	- [修改]  `generatePatchVerilog`，修復constant移除在wire宣告問題
	- [修改]  `generatePatchVerilog`，修復PO當成輸入輸出的宣告及命名方式
	- [新增]  `generatePatchVerilog`，產生cost計算

## To Do List
- [X] 利用Graph的Netlist做Structure Compare
- [X] 利用Random Simulation做Compare
- [X] 利用SAT做Compare
- [X]  產生patch
- [X] 解決 Random Simulation會造成Gate不見的問題(如:Meeting 06/21 pg.2)



## abc
### Compile 在src資料夾中
```bash
gcc -Wall -g -c check_blif.c -o blif2cnf.o
g++ -g -o blif2cnf.out blif2cnf.o ../libabc.a -lm -ldl -lreadline -lpthread
```
### 執行
```bash
./src/.exe filename 
```
## minisat
### Compile
```bash
make config prefix=$PREFIX(/home/usr/minisat)
make install
```


## How to Compile
```bash
export  LD_LIBRARY_PATH=$HOME/cada0047_beta/minisat-master/lib/
g++ --std=c++0x main.cpp -o cada0047_beta
```
