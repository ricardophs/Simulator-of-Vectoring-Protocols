import matplotlib.pyplot   as plt
import numpy as np
from scipy.interpolate import interp1d
import getopt, sys

metrics_opt = {"HC" : "HOPS",
               "SP" : "SP",
               "SWP" : "SWP",
               "WSP" : "WSP",
               "EM" : "COMPOSITE",
               "ALL" : "All"}

metrics_str = {"HOPS" : "Hop Count",
               "SP" : "Shortest Paths",
               "SWP" : "Shortest-Widest Paths",
               "WSP" : "Widest-Shortest Paths",
               "COMPOSITE" : "EIGRP Composite",
               'All' : 'All Metrics'}

type_str = {'ASYNC' : 'Asynchronous', 'SYNC' : 'Synchronous'}

Tests = {0: 'ADV', 1: 'W', 2: 'LF', 3: 'LF2', 4: 'LF2Conn'}
experiments_str = {'ADV': 'Announcement', 'W': 'Withdrawal', 'LF': 'Link Failure', 'LF2': 'Two Links Failure', 'LF2Conn': 'Two Links Failure Connected'}
experiments_folders = {'ADV': 'Advertises', 'W': 'Withdrawals', 'LF': 'Link Failures', 'LF2': 'Link Failures', 'LF2Conn': 'Link Failures'}

# use LaTeX fonts in the plot
plt.rc('font', weight='bold')
plt.rc('font', **{'family': 'sans-serif', 'sans-serif': ['Helvetica']})
plt.rc('text', usetex=True)
# plt.rc('font', family='serif')
# plt.rc('figure', figsize=np.multiply(1.2,plt.rcParams ["figure.figsize"]))
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"



OutputsFolder = "Outputs"
# Colors = ['mediumaquamarine', 'blueviolet', 'lightcoral', 'darkorange', 'royalblue', 'crimson', 
        #   'dimgray', 'navy', 'black', 'teal', 'hotpink', 'peru']
Colors = ['mediumaquamarine', 'peru', 'royalblue', 'crimson', 'lightcoral', 'darkorange', 'blueviolet',
          'dimgray', 'navy', 'black', 'teal', 'hotpink']
Linestyles = ['-', ':', (0, (3,1,2,1)), '--']
Linewidths = [2, 3, 2, 2]

def plotTerminationTimes(Network:str, Protocol:str, Metric:str, Type:str, Test:str):
    Metrics = [Metric]
    if Metric == 'All':
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
    Protocols = [Protocol + '_' + Type]
    if Protocol == 'All':
        Protocols = ['EIGRP_' + Type, 'BGP_' + Type]
        if Type == 'ASYNC':
            Protocols = ['EIGRP_' + Type, 'BGP_QuasiSync']
            # Protocols.append('BGP_QuasiSync')
    elif Protocol == 'FC':
        Type = 'ASYNC'
        Protocols = ['EIGRP_' + Type, 'EIGRPFC_' + Type ]
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
    elif Protocol == 'VP':
        Type = 'ASYNC'
        Protocols = ['BGP_' + Type, 'VP_' + Type]
        Metrics = ["HOPS", "SP", "WSP", "COMPOSITE", "SWP"]
    
    x, x_interp = [], []
    y, y_interp = [], []
    labels = []
    avg_time = 0

    aux = np.ones(len(Protocols) * len(Metrics), dtype=np.int8)
    for i, metric in enumerate(Metrics):
        for j, protocol in enumerate(Protocols):
            filename = "%s/%s/%s/%s_%s_TerminationTimes.txt" % (OutputsFolder, Network, metric, protocol, Test)
            try:
                with open(filename, "r") as f:
                    lines = f.read().splitlines()  

                first = lines[0].strip().split()
                N, E = int(first[0]), (int(first[0]) if len(first) == 1 else int(first[1]))

                times = np.zeros(N*E, dtype=np.int32)
                for d, line in enumerate(lines[1:]):   
                    times[d*N : (d+1)*N] = line.strip().split()[1:]

                if Type != "SYNC":
                    times //= 1000

                avg_time = np.mean(times)

                TimesBins = np.bincount(times)
                CCDF = 1 - np.divide(np.cumsum(TimesBins), np.sum(TimesBins), dtype=np.float32)

                x.append( np.arange( 0, CCDF.size, dtype=np.float32 ) )
                # if Type != "SYNC":
                #     x[-1] /= 1000
                #     avg_time /= 1000
                y.append( CCDF )

                if(metric in ['SWP']):
                    print(protocol, metric, "-")
                    print(x[-1][40],"\n",y[-1][40]*100)
                    print('stable in 0:', y[-1][0]*100)
                    print('max stabilisation time:', x[-1][-1])
                    print()

                fun = interp1d(x=x[-1], y=y[-1], kind=1, fill_value='extrapolate')
                x2_aux = np.linspace(start=min(x[-1]), stop=max(x[-1]), num=1000)
                x_interp.append(x2_aux)
                y_interp.append(fun(x2_aux))

                # label = protocol.replace('_', ' ')
                label = 'gEIGRP' if 'EIGRP' in protocol else 'cBGP'
                if 'FC' in protocol:
                    label += ' (FC)'
                if Metric == 'All':
                    label += f' {metric}' if metric not in ['COMPOSITE', 'HOPS'] else ' Hop Count' if metric == 'HOPS' else ' Composite'
                label += ' (%.1f %s)' % (avg_time, 'iterations' if Type == 'SYNC' else 'ms')
                labels.append( label )

            except Exception as e:
                print(f"Error in plotTerminationTimes ({Network}, {protocol}, {metric}, {Type}, {Test}):", e)
                aux[i*len(Protocols) + j] = 0
                x_interp.append(0)
                y_interp.append(0)
                labels.append("")
    
    for i, prt in enumerate(aux):    
        if prt:
            plt.plot( x_interp[i], y_interp[i], color = Colors[i//2], label = labels[i], linewidth = Linewidths[i%2], linestyle = Linestyles[i%2] )

    plt.legend( fontsize = 12 )
    # plt.title( rf"Stabilisation Times per Node - {metrics_str[Metric]} - {Network} - {experiments_str[Test]}",
    #            fontsize = 14)
    plt.title( rf"Stabilisation Times per Node - {Network} - {experiments_str[Test]}",
               fontsize = 14)
    labelx = r"Time [ms]" if Type != "SYNC" else r"Synchronnous Iterations"
    plt.xlabel( labelx , 
                fontsize = 12 , 
                labelpad = 10 )
    plt.ylabel( r"CCDF", 
                fontsize = 12 , 
                labelpad = 10 )
    plt.xticks( fontsize = 12 )
    plt.yticks( fontsize = 12 )

    plt.ylim( 0, 1.05 )
    plt.xlim( 0, 220 )

    plt.tight_layout()
    plt.grid( True, which = "both" )

def plotMessages(Network:str, Protocol:str, Metric:str, Type:str, Test:str):
    Metrics = [Metric]
    if Metric == 'All':
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
    Protocols = [Protocol + '_' + Type]
    if Protocol == 'All':
        Protocols = ['EIGRP_' + Type, 'BGP_' + Type, ]
        if Type == 'ASYNC':
            Protocols = ['EIGRP_' + Type, 'BGP_QuasiSync']
            # Protocols.append('BGP_QuasiSync')
    elif Protocol == 'FC':
        Type = 'ASYNC'
        Protocols = ['EIGRP_' + Type, 'EIGRPFC_' + Type ]
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
    elif Protocol == 'VP':
        Type = 'ASYNC'
        Protocols = ['BGP_' + Type, 'VP_' + Type]
        Metrics = ["HOPS", "SP", "WSP", "COMPOSITE", "SWP"]
    
    x, x_interp = [], []
    y, y_interp = [], []
    labels = []
    avgs = []

    aux = np.ones(len(Protocols) * len(Metrics), dtype=np.int8)
    for i, metric in enumerate(Metrics):
        for j, protocol in enumerate(Protocols):
            filename = "%s/%s/%s/%s_%s_Messages.txt" % (OutputsFolder, Network, metric, protocol, Test)
            try:
                with open(filename, "r") as f:
                    lines = f.read().splitlines()  

                first = lines[0].strip().split()
                N, E = int(first[0]), (int(first[0]) if len(first) == 1 else int(first[1]))

                msgs = np.zeros(3*E, dtype=np.int32)
                for d, line in enumerate(lines[1:]):   
                    msgs[d*3 : (d+1)*3] = line.strip().split()[1:4]

                MessagesBins = np.bincount(msgs[::3])
                CCDF = 1 - np.divide(np.cumsum(MessagesBins), np.sum(MessagesBins), dtype=np.float32)

                x.append( np.arange( 0, CCDF.size, dtype=np.float32 ) )
                y.append( CCDF )

                fun = interp1d(x=x[-1], y=y[-1], kind=1, fill_value='extrapolate')
                x2_aux = np.linspace(start=min(x[-1]), stop=max(x[-1]), num=1000)
                x_interp.append(x2_aux)
                y_interp.append(fun(x2_aux))

                # label = protocol.replace('_', ' ')
                label = 'gEIGRP' if 'EIGRP' in protocol else 'cBGP'
                if Metric == 'All':
                    label += f' {metric}' if metric not in ['COMPOSITE', 'HOPS'] else ' Hop Count' if metric == 'HOPS' else ' Composite'
                # avg = '(%d' %  np.mean(msgs[::3])
                # avg += ' ; %d)' %  np.mean(msgs[1::3]) if 'EIGRP' in protocol else ')'
                label += ' (%d' %  np.mean(msgs[::3])
                label += ' ; %d)' %  np.mean(msgs[1::3]) if 'EIGRP' in protocol else ')'
                labels.append( label )
                # avgs.append( avg )

            except Exception as e:
                print(f"Error in plotMessages ({Network}, {protocol}, {metric}, {Type}, {Test}):", e)
                aux[i*len(Protocols) + j] = 0
                x_interp.append(0)
                y_interp.append(0)
                labels.append("")
    
    for i, prt in enumerate(aux):    
        if prt:
            plt.plot( x_interp[i], y_interp[i], color = Colors[i//2], label = labels[i], linewidth = Linewidths[i%2], linestyle = Linestyles[i%2] )

    # legends = [f'{label:16} {avg:>11}' for label,avg in zip(labels, avgs)]

    # plt.legend( legends, fontsize = 12)
    plt.legend( fontsize = 12)
    # plt.title( rf"Messages Exchanged - {metrics_str[Metric]} - {Network} - {experiments_str[Test]}",
    #            fontsize = 12)
    plt.title( rf"Messages Exchanged - {Network} - {experiments_str[Test]}",
               fontsize = 14)
    labelx = r"Number of messages"
    plt.xlabel( labelx , 
                fontsize = 12 , 
                labelpad = 10 )
    plt.ylabel( r"CCDF", 
                fontsize = 12 , 
                labelpad = 10 )
    plt.xticks( fontsize = 12 )
    plt.yticks( fontsize = 10 )

    plt.ylim( 0, 1.05 )
    plt.xlim( 0, 4e4)

    plt.tight_layout()
    plt.grid( True, which = "both" )

def plotAttributeChangesCount(Network:str, Protocol:str, Metric:str, Type:str, Test:str):
    Metrics = [Metric]
    if Metric == 'All':
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
    Protocols = [Protocol + '_' + Type]
    if Protocol == 'All':
        Protocols = ['EIGRP_' + Type, 'BGP_' + Type, ]
        if Type == 'ASYNC':
            Protocols = ['EIGRP_' + Type, 'BGP_QuasiSync']
            # Protocols.append('BGP_QuasiSync')
    elif Protocol == 'FC':
        Type = 'ASYNC'
        Protocols = ['EIGRP_' + Type, 'EIGRPFC_' + Type ]
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
    elif Protocol == 'VP':
        Type = 'ASYNC'
        Protocols = ['BGP_' + Type, 'VP_' + Type]
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
    
    x = []
    y = []
    labels = []

    aux = np.ones(len(Protocols) * len(Metrics), dtype=np.int8)
    for i, metric in enumerate(Metrics):
        for j, protocol in enumerate(Protocols):
            filename = "%s/%s/%s/%s_%s_AttributeChangesCount.txt" % (OutputsFolder, Network, metric, protocol, Test)
            try:
                with open(filename, "r") as f:
                    lines = f.read().splitlines()  

                first = lines[0].strip().split()
                N, E = int(first[0]), (int(first[0]) if len(first) == 1 else int(first[1]))

                changes = np.zeros(N*E, dtype=np.int32)
                for d, line in enumerate(lines[1:]):   
                    changes[d*N : (d+1)*N] = line.strip().split()[1:]

                ChangesBins = np.bincount(changes)
                CCDF = 1 - np.divide(np.cumsum(ChangesBins), np.sum(ChangesBins), dtype=np.float32)

                CCDF = np.append(CCDF, 0)
                CCDF[2:] = CCDF[0:-2]
                CCDF[0:2] = np.ones(2)
                CCDF = np.append(CCDF, 0)

                x.append( np.arange( -1, CCDF.size-1, dtype=np.int32 ) )
                y.append( CCDF )

                label = protocol.replace('_', ' ')
                if Metric == 'All':
                    label += rf' {metric}'
                label += ' (%.1f)' % np.mean(changes)
                labels.append( label )

            except Exception as e:
                print(f"Error in plotAttributeChangesCount ({Network}, {protocol}, {metric}, {Type}, {Test}):", e)
                aux[i*len(Protocols) + j] = 0
                x.append( [] )
                y.append( [] )
                labels.append("")
    
    for i, prt in enumerate(aux):    
        if prt:
            plt.step( x[i], y[i], drawstyle="steps-pre", color = Colors[i//2], label = labels[i], linewidth = Linewidths[i%2], linestyle = Linestyles[i%2] )

    plt.legend( fontsize = 10 )
    plt.title( rf"Attribute Changes per Node - {metrics_str[Metric]} - {Network} - {experiments_str[Test]}",
               fontsize = 12)
    labelx = r"Number of attribute changes"
    plt.xlabel( labelx , 
                fontsize = 10 , 
                labelpad = 10 )
    plt.ylabel( r"CCDF", 
                fontsize = 10 , 
                labelpad = 10 )
    plt.xticks( fontsize = 10 )
    plt.yticks( fontsize = 10 )

    plt.ylim( 0, 1.05 )
    _, r = plt.xlim()
    plt.xlim(left=-0.05*r)

    plt.tight_layout()
    plt.grid( True, which = "both" )

def plotTransitions(Network:str, Metric:str, Type:str, Test:str):
    Metrics = [Metric]
    Protocols = ['EIGRP' + '_' + Type]
    if Metric == 'All':
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
    elif Metric == 'FC':
        Type = 'ASYNC'
        Protocols = ['EIGRP_' + Type, 'EIGRPFC_' + Type ]
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
        Metric = 'All'
    
    x = []
    y = []
    labels = []

    aux = np.ones(len(Protocols) * len(Metrics), dtype=np.int8)
    for i, metric in enumerate(Metrics):
        for j, protocol in enumerate(Protocols):
            filename = "%s/%s/%s/%s_%s_Transitions.txt" % (OutputsFolder, Network, metric, protocol, Test)
            try:
                with open(filename, "r") as f:
                    lines = f.read().splitlines()  

                first = lines[0].strip().split()
                N, E = int(first[0]), (int(first[0]) if len(first) == 1 else int(first[1]))

                transitions = np.zeros(N*E, dtype=np.int32)
                for d, line in enumerate(lines[1:]):   
                    transitions[d*N : (d+1)*N] = line.strip().split()[1:]

                TransitionsBins = np.bincount(transitions)
                CCDF = 1 - np.divide(np.cumsum(TransitionsBins), np.sum(TransitionsBins), dtype=np.float32)

                CCDF = np.append(CCDF, 0)
                CCDF[2:] = CCDF[0:-2]
                CCDF[0:2] = np.ones(2)
                CCDF = np.append(CCDF, 0)

                x.append( np.arange( -1, CCDF.size-1, dtype=np.int32 ) )
                y.append( CCDF )

                label = protocol.replace('_', ' ')
                if Metric == 'All':
                    label += rf' {metric}'
                label += ' (%.1f)' % np.mean(transitions)
                labels.append( label )

            except Exception as e:
                print(f"Error in plotTransitions ({Network}, {protocol}, {metric}, {Type}, {Test}):", e)
                aux[i*len(Protocols) + j] = 0
                x.append([])
                y.append([])
                labels.append("")
    
    for i, prt in enumerate(aux):    
        if prt:
            plt.step( x[i], y[i], drawstyle="steps-pre", color = Colors[i//2], label = labels[i], linewidth = Linewidths[i%2], linestyle = Linestyles[i%2] )

    plt.legend( fontsize = 10 )
    plt.title( rf"U$\rightarrow$D state transitions per Node - {metrics_str[Metric]} - {Network} - {experiments_str[Test]}",
               fontsize = 12)
    labelx = r"Number of update to diffusing state transitions"
    plt.xlabel( labelx , 
                fontsize = 10 , 
                labelpad = 10 )
    plt.ylabel( r"CCDF", 
                fontsize = 10 , 
                labelpad = 10 )
    plt.xticks( fontsize = 10 )
    plt.yticks( fontsize = 10 )

    plt.ylim( 0, 1.05 )
    _, r = plt.xlim()
    plt.xlim(left=-0.05*r)

    plt.tight_layout()
    plt.grid( True, which = "both" )

def plotCycleBlackHoleTimes(Network:str, Protocol:str, Metric:str, Type:str, Test:str):
    Metrics = [Metric]
    if Metric == 'All':
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
    Protocols = [Protocol + '_' + Type]
    if Protocol == 'All':
        Protocols = ['EIGRP_' + Type, 'BGP_' + Type, ]
        if Type == 'ASYNC':
            Protocols = ['EIGRP_' + Type, 'BGP_QuasiSync']
            # Protocols.append('BGP_QuasiSync')
    elif Protocol == 'FC':
        Type = 'ASYNC'
        Protocols = ['EIGRP_' + Type, 'EIGRPFC_' + Type ]
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
    elif Protocol == 'VP':
        Type = 'ASYNC'
        Protocols = ['BGP_' + Type, 'VP_' + Type]
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
    
    x, x_interp = [], []
    y, y_interp = [], []
    labels = []
    avg_time = 0
    data = []

    aux = np.ones(len(Protocols) * len(Metrics), dtype=np.int8)
    for i, metric in enumerate(Metrics):
        for j, protocol in enumerate(Protocols):
            filename = "%s/%s/%s/%s_%s_CycleBlackHoleTimes.txt" % (OutputsFolder, Network, metric, protocol, Test)
            try:
                with open(filename, "r") as f:
                    lines = f.read().splitlines()  

                first = lines[0].strip().split()
                N, E = int(first[0]), (int(first[0]) if len(first) == 1 else int(first[1]))

                times = np.empty((0,0), dtype=np.float32)
                for line in lines[1:]:   
                    times = np.append(times, np.array(list(filter(lambda x : x != '-1', line.strip().split()[1:])), dtype=np.float32))

                # print(times)
                if len(times) == 0: 
                    # print(f"Error in plotCycleBlackHoleTimes ({Network}, {Protocol}, {Metric}, {Type}, {Test}):", e)
                    raise("Not Enough Data!")
                    # return

                times = np.array(times, np.int32)
                
                if Type != "SYNC":
                    times //= 1000

                data.append(times)

                avg_time = np.mean(times)

                TimesBins = np.bincount(times)
                CCDF = 1 - np.divide(np.cumsum(TimesBins), np.sum(TimesBins), dtype=np.float32)

                x.append( np.arange( 0, CCDF.size, dtype=np.float32 ) )
                y.append( CCDF )

                if("BGP" in protocol and metric in ['COMPOSITE', 'SWP']):
                    print(protocol, metric, "-")
                    print(x[-1],"\n",y[-1])
                    print('max time in black-hole time:', x[-1][-1])
                    print()

                fun = interp1d(x=x[-1], y=y[-1], kind=2, fill_value='extrapolate')
                x2_aux = np.linspace(start=min(x[-1]), stop=max(x[-1]), num=1000)
                x_interp.append(x2_aux)
                y_interp.append(fun(x2_aux))

                # label = protocol.replace('_', ' ')
                label = 'gEIGRP' if 'EIGRP' in protocol else 'cBGP'
                if Metric == 'All':
                    label += f' {metric}' if metric not in ['COMPOSITE', 'HOPS'] else ' Hop Count' if metric == 'HOPS' else ' Composite'
                label += ' (%.1f %s)' % (avg_time, 'iterations' if Type == 'SYNC' else 'ms')
                labels.append( label )

            except Exception as e:
                print(f"Error in plotCycleBlackHoleTimes ({Network}, {protocol}, {metric}, {Type}, {Test}):", e)
                aux[i*len(Protocols) + j] = 0
                x_interp.append(0)
                y_interp.append(0)
                labels.append("")
    
    for i, prt in enumerate(aux):    
        if prt:
            plt.plot( x_interp[i], y_interp[i], color = Colors[i//2], label = labels[i], linewidth = Linewidths[i%2], linestyle = Linestyles[i%2] )
            # plt.boxplot(data, labels=labels)

    plt.legend( fontsize = 12 )
    measured = 'Black Hole' if Protocol == 'EIGRP' else ('Cycle' if Protocol == 'BGP' else 'Black Hole/Cycle')
    # plt.title( rf"Time In a {measured} per Node - {metrics_str[Metric]} - {Network} - {experiments_str[Test]}",
    #            fontsize = 12)    
    plt.title( rf"Time In a {measured} per Node - {Network} - {experiments_str[Test]}",
               fontsize = 14)
    labelx = r"Time [ms]" if Type != "SYNC" else r"Synchronnous Iterations"
    plt.xlabel( labelx , 
                fontsize = 12 , 
                labelpad = 10 )
    plt.ylabel( r"CCDF", 
                fontsize = 12 , 
                labelpad = 10 )
    plt.xticks( fontsize = 12 )
    plt.yticks( fontsize = 12 )

    plt.ylim( 0, 1.05 )
    plt.xlim( 0, 1e3)

    plt.tight_layout()
    plt.grid( True, which = "both" )

def plotStableStateTimes(Network:str, Protocol:str, Metric:str, Type:str, Test:str):
    Metrics = [Metric]
    if Metric == 'All':
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
    Protocols = [Protocol + '_' + Type]
    if Protocol == 'All':
        Protocols = ['EIGRP_' + Type, 'BGP_' + Type]
        if Type == 'ASYNC':
            Protocols = ['EIGRP_' + Type, 'BGP_QuasiSync']
            # Protocols.append('BGP_QuasiSync')
    elif Protocol == 'FC':
        Type = 'ASYNC'
        Protocols = ['EIGRP_' + Type, 'EIGRPFC_' + Type ]
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
    elif Protocol == 'VP':
        Type = 'ASYNC'
        Protocols = ['BGP_' + Type, 'VP_' + Type]
        Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
    
    x, x_interp = [], []
    y, y_interp = [], []
    labels = []
    avg_time = 0

    aux = np.ones(len(Protocols) * len(Metrics), dtype=np.int8)
    for i, metric in enumerate(Metrics):
        for j, protocol in enumerate(Protocols):
            filename = "%s/%s/%s/%s_%s_StableStateTimes.txt" % (OutputsFolder, Network, metric, protocol, Test)
            try:
                with open(filename, "r") as f:
                    lines = f.read().splitlines()  

                first = lines[0].strip().split()
                N, E = int(first[0]), (int(first[0]) if len(first) == 1 else int(first[1]))

                times = np.zeros(E, dtype=np.int32)
                for d, line in enumerate(lines[1:]):   
                    times[d] = line.strip().split()[1]

                if Type != "SYNC":
                    times //= 1000

                avg_time = np.mean(times)

                TimesBins = np.bincount(times)
                CCDF = 1 - np.divide(np.cumsum(TimesBins), np.sum(TimesBins), dtype=np.float32)

                x.append( np.arange( 0, CCDF.size, dtype=np.float32 ) )
                # if Type != "SYNC":
                #     x[-1] /= 1000
                #     avg_time /= 1000
                y.append( CCDF )

                if("BGP" in protocol and metric in ['COMPOSITE', 'SWP']):
                    print(protocol, metric, "-")
                    print(x[-1],"\n",y[-1])
                    print('max termination time:', x[-1][-1])
                    print()

                fun = interp1d(x=x[-1], y=y[-1], kind=1, fill_value='extrapolate')
                x2_aux = np.linspace(start=min(x[-1]), stop=max(x[-1]), num=1000)
                x_interp.append(x2_aux)
                y_interp.append(fun(x2_aux))

                # label = protocol.replace('_', ' ')
                label = 'gEIGRP' if 'EIGRP' in protocol else 'cBGP'
                if Metric == 'All':
                    label += f' {metric}' if metric not in ['COMPOSITE', 'HOPS'] else ' Hop Count' if metric == 'HOPS' else ' Composite'
                label += ' (%.1f %s)' % (avg_time, 'iterations' if Type == 'SYNC' else 'ms')
                labels.append( label )

            except Exception as e:
                print(f"Error in plotStableStateTimes ({Network}, {protocol}, {metric}, {Type}, {Test}):", e)
                aux[i*len(Protocols) + j] = 0
                x_interp.append(0)
                y_interp.append(0)
                labels.append("")
    
    for i, prt in enumerate(aux):    
        if prt:
            plt.plot( x_interp[i], y_interp[i], color = Colors[i//2], label = labels[i], linewidth = Linewidths[i%2], linestyle = Linestyles[i%2] )

    plt.legend( fontsize = 12 )
    # plt.title( rf"Stable State Times - {metrics_str[Metric]} - {Network} - {experiments_str[Test]}",
    #            fontsize = 12)
    plt.title( rf"Termination Time - {Network} - {experiments_str[Test]}",
               fontsize = 12)
    labelx = r"Time [ms]" if Type != "SYNC" else r"Synchronnous Iterations"
    plt.xlabel( labelx , 
                fontsize = 12 , 
                labelpad = 10 )
    plt.ylabel( r"CCDF", 
                fontsize = 12 , 
                labelpad = 10 )
    plt.xticks( fontsize = 12 )
    plt.yticks( fontsize = 10 )

    plt.ylim( 0, 1.05 )
    plt.xlim( 0, )

    plt.tight_layout()
    plt.grid( True, which = "both" )


def usage(args):
    print(f'usage: {args[0]} [-h] [-p PROTOCOL] [-m METRIC] [-t BEHAVIOUR] [-e EXPERIMENT] [--save] [--noshow] [--saveall] NETWORK')
    print('\nPlots the curves for the simulation results, according to user input')
    print(f'Example usage: {args[0]} -p All -m SP -t ASYNC -e W --save Abilene')
    print('Plots the curves for termination times, number of messages exchanged, number of attribute \
changes, and number of update to diffusing state transitions for both EIGRP and BGP protocols, \
operating asynchronously under the shortest paths metric, in the Abilene network. Also saves the plots to the \'Plots\ folder\'.')
    
    print('\npositional arguments:')
    print('NETWORK                           the name of the network to which the plots pertain')
    
    print('\noptions:')
    print('-h, --help                        show this help message and exit\n')

    print('-p, --protocol PROTOCOL           protocol for which the plots are generated. \'All\' \
if both EIGRP and BGP are to be considered (options: \'EIGRP\', \'BGP\', \
\'All\'; default: \'EIGRP\')\n')

    print('-m, --metric METRIC               routing metric for which the plots are generated. \'All\' \
if all metrics are to be considered (options: \'HC\', \'SP\', \'SWP\', \'EM\', \'WSP\' ; \
default: \'HC\')\n')

    print('-t, --type BEHAVIOUR              protocol behaviour for which the plots are generated. \
(options: \'SYNC\', \'ASYNC\', \'QUASI\' ; \
default: \'SYNC\')\n')

    print('-e, --experiment EXPERIMENT       experiment for which the plots are generated. \
(options: \'ADV\', \'W\', \'LF\', \'LF2\' ; \
default: \'W\')\n')

    print('--save                            if present, the plots are saved to the Plots\ folder \
(default: False)\n')

    print('--noshow                          if present, the plots are not shown to the user. \
may be used in conjunction with \'--save\' (default: False)\n')

    print('--saveall                         if present, saves all plots for NETWORK network, \
without showing them (default: False)\n')

    print('\n')
               

def main():
    # defaults
    network = 'Abilene'
    protocol = 'All'
    metric = 'HOPS'
    type = 'SYNC'
    experiment = 'W'
    show, save = True, False
    saveAll = False
    # get arguments
    try:
        shorts="hp:m:t:e:"
        longs=["help", "protocol=", "type=", "metric=", "experiment=", "save", "noshow", "saveall"]
        opts, args = getopt.getopt(sys.argv[1:], shorts, longs)
    except getopt.GetoptError as err:
        print(err) 
        usage(sys.argv)
        sys.exit(2)
    # Network argument
    if len(args) == 1:
        network = args[0]
    elif len(args) == 0:
        pass
    else:
        print("Invalid network")
        usage(sys.argv)
        sys.exit()
    # process arguments
    for o, a in opts:
        if o in ("-h", "--help"):
            usage(sys.argv)
            sys.exit(0)
        elif o in ("-p", "--protocol"):
            if a.upper() not in ('EIGRP', 'BGP', "ALL"):
                print("Invalid argument following", o)
                sys.exit(1)
            protocol = a.upper() if a.upper() != 'ALL' else "All"
        elif o in ("-t", "--type"):
            if a.upper() not in ('SYNC', 'ASYNC', 'QUASI'):
                print("Invalid argument following", o)
                sys.exit(1)
            type = a.upper() if a.upper() != 'QUASI' else "QuasiSYNC"
        elif o in ("-m", "--metric"):
            if a.upper() not in ('SP', 'SWP', "WSP", 'HC', 'EM', 'ALL'):
                print("Invalid argument following", o)
                sys.exit(1)
            metric = metrics_opt[a.upper()]
        elif o in ("-e", "--experiment"):
            if a.upper() not in ('ADV', 'W', 'LF', 'LF2'):
                print("Invalid argument following", o)
                sys.exit(1)
            experiment = a.upper()
        elif o in ("--save"):
            save = True
        elif o in ("--noshow"):
            show = False
        elif o in ("--saveall"):
            saveAll = True
            show = False
            save = True
            protocol = 'All'
        else:
            assert False, "unhandled option"

    if saveAll:
        print("Saving all plots for Network", network)
    else:
        print("Network:", network, "\nProtocol:", protocol, 
              "\nMetric:", metrics_str[metric], "\nBehaviour:", type_str[type], 
              "\nExperiment:", experiments_str[experiment])

    if saveAll:
        # Types = ["SYNC", "ASYNC"]
        Types = ["ASYNC"]
        # Metrics = ["HOPS", "SP", "WSP","COMPOSITE", "SWP"]
        Metrics = ["All"]
        # Experiments = ["ADV", "W", "LF", "LF2"]
        # Experiments = ["ADV"]
        Experiments = ["LF"]
        # Experiments = ["ADV", "LF", "LF2"]
    else:
        Types = [type]
        Metrics = [metric]
        Experiments = [experiment]
    
    for tp in Types:
        for metr in Metrics:
            for exp in Experiments:

                nameFuncAbilene = lambda e : "Plots/Abilene/%s/%s_%s%s_%s%s%s.png" % \
                                    ( e,
                                      network, 
                                      protocol + "_" if protocol != 'All' else '',
                                      tp,
                                      metr + "_" if metr != 'All' else '', 
                                      e,
                                      exp )

                nameFuncASES = lambda e : "Plots/%s/%s/%s_%s%s_%s%s%s.png" % \
                                    ( experiments_folders[exp],
                                      e,
                                      network, 
                                      protocol + "_" if protocol != 'All' else '',
                                      tp,
                                      metr + "_" if metr != 'All' else '', 
                                      e,
                                      exp )
                
                # nameFunc = nameFuncAbilene if network == 'Abilene' else nameFuncASES

                # nameFunc = lambda e : "Plots/ComparisonVPvsBGP/%s/%s_ASYNC_%s.png" % (e, net, exp)
                nameFunc = lambda e : "Plots/ComparisonFCvsNoFC/%s/%s_ASYNC_%s.png" % (e, network, exp)

                # plt.figure()
                # plotTerminationTimes(network, protocol, metr, tp, exp)
                # fileNamePlot1 = nameFunc("TerminationTimes")
                # if save:
                #     plt.savefig( fileNamePlot1 )
                
                plt.figure()
                plotTerminationTimes(network, 'FC', metr, tp, exp)
                fileNamePlot1 = nameFunc("TerminationTimes")
                if save:
                    plt.savefig( fileNamePlot1 )

                # plt.figure()
                # plotMessages(network, protocol, metr, tp, exp)
                # fileNamePlot2 = nameFunc("Messages")
                # if save:
                #     plt.savefig( fileNamePlot2 )

                # plt.figure()
                # plotAttributeChangesCount(network, protocol, metr, tp, exp)
                # fileNamePlot3 = nameFunc("AttributeChanges")
                # if save:
                #     plt.savefig( fileNamePlot3 )

                # if protocol in ('EIGRP', 'All'):
                #     metr_aux = 'All' if saveAll else metr
                #     plt.figure()
                #     plotTransitions(network, metr_aux, tp, exp)
                #     fileNamePlot4 = nameFunc("Transitions")
                #     if save:
                #         plt.savefig( fileNamePlot4 )


                # if(exp in ('LF', 'LF2') or (metr in ('COMPOSITE', 'SWP', 'All') and exp == 'ADV')):
                #     plt.figure()
                #     plotCycleBlackHoleTimes(network, protocol, metr, tp, exp)
                #     fileNamePlot5 = nameFunc("CycleBlackHoleTimes")
                #     if save:
                #         plt.savefig( fileNamePlot5 )

                # plt.figure()
                # plotStableStateTimes(network, protocol, metr, tp, exp)
                # fileNamePlot6 = nameFunc("StableStateTimes")
                # if save:
                #     plt.savefig( fileNamePlot6 )

                if show:
                    plt.show()
                plt.close('all')


if __name__ == "__main__":
    main()
    exit(0)


    # Experiments = ["ADV", "W", "LF", "LF2"]
    Experiments = ["ADV"]
    Networks = ["Abilene", "AS1221", "AS1755", "AS3257", "AS3967", "AS6461", "AS1239"]
    # Networks = ["AS3257", "AS6461"]
    # Networks = ["AS1239"]
    # Networks = ["Abilene"]

    for net in Networks:
        for exp in Experiments:
            nameFunc = lambda e : "Plots/ComparisonVPvsBGP/%s/%s_ASYNC_%s.png" % (e, net, exp)

            # plt.figure()
            # plotTerminationTimes(net, "VP", "All", "ASYNC", exp)
            # fileNamePlot = nameFunc("TerminationTimes")
            # plt.savefig(fileNamePlot)
            # plt.show()

            # plt.figure()
            # plotMessages(net, "VP", "All", "ASYNC", exp)
            # fileNamePlot = nameFunc("Messages")
            # plt.savefig(fileNamePlot)
            # plt.show()

            # plt.figure()
            # plotAttributeChangesCount(net, "VP", "All", "ASYNC", exp)
            # fileNamePlot = nameFunc("AttributeChanges")
            # plt.savefig(fileNamePlot)
            # plt.show()

            # plt.figure()
            # plotTransitions(net, "FC", "ASYNC", exp)
            # fileNamePlot = nameFunc("Transitions")
            # plt.savefig(fileNamePlot)
            # # plt.show()

            # if(exp in ['LF', 'LF2']):
            plt.figure()
            plotCycleBlackHoleTimes(net, "VP", "All", "ASYNC", exp)
            fileNamePlot = nameFunc("CycleBlackHoleTimes")
            plt.savefig(fileNamePlot)
            # plt.show()

            # plt.show()
            # exit(0)
            plt.close('all')


