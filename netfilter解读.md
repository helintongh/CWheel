# 1.介绍

netfilter是一个包篡改(修改，过滤等操作)的框架，在普通的Berkeley套接字接口之外。它有四个部分。

首先，每个协议定义“钩子”(IPv4定义5)，这些钩子是定义良好的包遍历协议栈的点。在每个点上，协议将使用包和钩子对应的号调用netfilter框架。

其次，内核的某些部分可以注册来监听每个协议的不同钩子。因此，当一个数据包被传递到netfilter框架时，它会检查是否有人注册了该协议和钩子(与epoll和select的回调函数相似);如果是这样，他们每个人都有机会按顺序检查(也可能改变)数据包，然后进行4种操作:丢弃这个包(NF_DROP)，允许它通过(NF_ACCEPT)，告诉netfilter忘记这个包(NF_STOLEN)，或者要求netfilter为用户空间(NF_QUEUE)对包进行排队。

第三部分是收集已经排队的数据包(通过ip_queue驱动程序)发送给用户空间;这些包是异步处理的。

除了这个原始框架之外，还编写了各种模块，这些模块提供了与以前的(前netfilter)内核类似的功能，特别是可扩展的NAT系统和可扩展的包过滤系统(iptables)。

# 2.netfilter架构

Netfilter仅仅是协议栈中不同点上的一系列钩子。(理想化的netfilter去遍历)IPv4遍历图如下:

```shell
一个通过Netfilter系统的包:
   --->[1]--->[ROUTE]--->[3]--->[4]--->
                 |            ^
                 |            |
                 |         [ROUTE]
                 v            |
                [2]          [5]
                 |            ^
                 |            |
                 v            |

```
左边是信息包的位置:通过了简单的完整性检查(即它们被传递给netfilter框架的NF_IP_PRE_ROUTING[1]钩子。

接下来，他们进入路由处理代码，该代码决定包是发送到另一个接口，还是本地进程。路由代码可能会丢弃无法路由的封包。

如果它的目的地是机器本身，在将netfilter框架传递给进程(如果有的话)之前，将再次调用NF_IP_LOCAL_IN[2]钩子。

如果它注定要传递到另一个接口，则为NF_IP_FORWARD[3]钩子调用netfilter框架。

然后，包通过最后一个netfilter钩子NF_IP_POST_ROUTING[4]钩子，然后再次上线。

为本地创建的包调用NF_IP_LOCAL_OUT[5]钩子。

这里你可以看到路由发生在这个钩子被调用后:实际上，首先调用路由代码(找出源IP地址和一些IP选项):如果你想改变路由，你必须自己改变' skb->dst'字段，就像在NAT代码中做的那样。

## 2.1 Netfilter基本

有一个用于IPv4的netfilter示例，可以看到每个钩子何时被激活。这就是netfilter的本质。

内核模块可以注册来监听这些钩子中的任何一个。注册函数的模块必须在钩子中指定函数的优先级;然后，当从核心网络代码调用netfilter钩子时，此时注册的每个模块按优先级顺序被调用，并且可以自由地操作数据包。模块可以告诉netfilter做以下五件事之一:

1. NF_ACCEPT:继续正常遍历。
2. NF_DROP:删除数据包;不要继续遍历。
3. NF_STOLEN:我已经接管了数据包;不要继续遍历。
4. NF_QUEUE:对数据包(通常用于用户空间处理)进行排队。
5. NF_REPEAT:再次调用这个钩子。

netfilter的其他部分(处理排队的包、很酷的注释)将在后面的内核部分中介绍。

在此基础上，可以构建相当复杂的包操作，如下面的两个部分所示。

## 2.2 包选择器:IP Tables

在netfilter框架上构建了一个称为IP表的包选择系统。它是ipchains(来自ipfwadm，来自BSD的ipfw IIRC)的直接派生，具有可扩展性。内核模块可以注册一个新表，并请求一个数据包遍历给定的表。这个包选择方法用于包过滤(“filter”表), 网络地址转换(“nat”表)和常规路由前数据包错误处理(“mangle”表)。

在netfilter中注册的钩子如下(每个钩子中的函数按实际调用的顺序排列):

```shell

   --->PRE------>[ROUTE]--->FWD---------->POST------>
       Conntrack    |       Mangle   ^    Mangle
       Mangle       |       Filter   |    NAT (Src)
       NAT (Dst)    |                |    Conntrack
       (QDisc)      |             [ROUTE]
                    v                |
                    IN Filter       OUT Conntrack
                    |  Conntrack     ^  Mangle
                    |  Mangle        |  NAT (Dst)
                    v                |  Filter
```

**数据包过滤**

表“filter”永远不应该改变包:只过滤它们。

与ipchains相比，iptables filter的优点之一是体积小，速度快，它通过NF_IP_LOCAL_IN、NF_IP_FORWARD和NF_IP_LOCAL_OUT点连接到netfilter。这意味着对于任何给定的包，都有一个(而且只有一个)可能的地方来过滤它。对于用户来说，这比ipchains要简单得多。另外，netfilter框架为NF_IP_FORWARD钩子提供了输入和输出接口，这意味着许多类型的过滤要简单得多。

**NAT**

这是“nat”表的领域，它由两个netfilter钩子提供数据包:对于非本地包，NF_IP_PRE_ROUTING和NF_IP_POST_ROUTING挂钩分别适用于目的地和源更改。如果定义了CONFIG_IP_NF_NAT_LOCAL，则使用钩子NF_IP_LOCAL_OUT和NF_IP_LOCAL_IN来更改本地包的目的地。

这个表与‘filter’表稍有不同，因为只有新连接的第一个数据包会遍历表:然后，这个遍历的结果应用于同一连接中所有未来的包。

**包伪装，端口转发，透明代理**

把NAT分成源NAT(SNAT第一个数据包的源地址改变了)和目的NAT(DNAT第一个数据包的目的地址改变了)。

伪装是源NAT的一种特殊形式:端口转发和透明代理是目的NAT的一种特殊形式。

**包篡改**

包篡改表(“mangle”表)用于实际更改包信息。示例应用程序是TOS和TCPMSS目标。把所有五个netfilter挂钩。

注:服务类型(ToS)字段是IPv4头的第二个字节。ToS字段可以指定数据报的优先级，并为低延迟、高吞吐量或高可靠的服务请求路由。

TCPMSS是TCP最大分段大小。

## 2.3连接跟踪

连接跟踪是NAT的基础，但它是作为一个单独的模块实现的;这允许包过滤代码的扩展来简单而干净地使用连接跟踪(“state”模块)。

# 3.netfilter编程框架

## 3.1理解NAT

NAT被分为连接跟踪(根本不操作数据包)和NAT代码本身。连接跟踪也被设计成由iptables模块使用，因此它可以在NAT不关心的状态中进行细微的区分。

**连接跟踪**

连接跟踪钩子连接到高优先级的NF_IP_LOCAL_OUT和NF_IP_PRE_ROUTING钩子，以便在数据包进入系统之前看到它们。

(注:socket buffer是数据包结构)
skb中的nfct字段是指向结构ip_conntrack内部的一个指针，位于infos[]数组中的一个。因此，我们可以通过它指向数组中的哪个元素来判断skb的状态:这个指针对状态结构和skb与那个状态的关系进行编码。

提取' nfct'字段的最佳方法是调用' ip_conntrack_get()'，如果没有设置它，它将返回NULL，或者连接指针，并填充描述包与该连接的关系的ctinfo。这个枚举类型有几个值:

**IP_CT_ESTABLISHED**
- 该数据包是按原方向建立的连接的一部分。

**IP_CT_RELATED**
-  数据包与连接相关，并按原方向传递。

**IP_CT_NEW**
- 数据包试图创建一个新的连接(显然，它是在原方向)。

**IP_CT_ESTABLISHED + IP_CT_IS_REPLY**
- 数据包在应答方向，包是已建立的连接的一部分。

**IP_CT_RELATED + IP_CT_IS_REPLY**
- 数据包与连接相关，并在应答方向传递。

因此，可以通过测试>= IP_CT_IS_REPLY来识别应答包。