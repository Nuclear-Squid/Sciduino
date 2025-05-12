import QtQuick
import QtCharts


ChartView {
    id: chartView

    animationOptions: ChartView.NoAnimation
    theme: ChartView.ChartThemeDark

    ValueAxis {
        id: axisX
        min: 0
        max: 500
    }

    ValueAxis {
        id: axisY
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
        var series = chartView.createSeries(ChartView.SeriesTypeLine, "signal", axisX, axisY);
        for (const [x, y] of values) series.append(x, y)
    }
}
