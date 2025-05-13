import QtQuick
import QtCharts


ChartView {
    id: chartView

    antialiasing: true
    animationOptions: ChartView.NoAnimation

    ValueAxis {
        id: axisX
        titleText: "Time [s]"
    }

    ValueAxis {
        id: axisY
        titleText: "Voltage [V]"
        min: -1
        max: 4
    }

    LineSeries {
        id: lineSeries
        name: "signal"
        axisX: axisX
        axisY: axisY
        useOpenGL: true
    }

    function renderGraph(values) {
        chartView.removeAllSeries();
        axisX.max = values[values.length - 1][0]
        axisX.applyNiceNumbers()
        var series = chartView.createSeries(ChartView.SeriesTypeLine, "signal", axisX, axisY);
        for (const [x, y] of values) series.append(x, y)
    }
}
