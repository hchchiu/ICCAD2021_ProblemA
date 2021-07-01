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

[img](https://github.com/hchchiu/ICCAD2021_ProblemA/blob/master/doc/case1.png)
## To Do List
-[] 利用Graph的Netlist做Structur Compare
-[] 利用Random Simulation做Compare
-[] 利用SAT做Compare

