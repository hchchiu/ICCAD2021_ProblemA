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



## To Do List
- [O] 利用Graph的Netlist做Structure Compare
- [ ] 利用Random Simulation做Compare
- [ ] 利用SAT做Compare

