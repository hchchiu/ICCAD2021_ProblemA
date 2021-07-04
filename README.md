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
## To Do List
- [X] 利用Graph的Netlist做Structure Compare
- [ ] 利用Random Simulation做Compare
- [ ] 利用SAT做Compare


## 執行abc
```bash
./src/.exe filename 
```
## 執行minisat
```bash
export  LD_LIBRARY_PATH="/usr/local/lib" 
```