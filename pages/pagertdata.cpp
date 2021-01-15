/*
    Original copyright 2018 Benjamin Vedder benjamin@vedder.se and the VESC Tool project ( https://github.com/vedderb/vesc_tool )
    Now forked to:
    Danny Bokma github@diebie.nl

    This file is part of BMS Tool.

    DieBieMS Tool is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DieBieMS Tool is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pagertdata.h"

PageRtData::PageRtData(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageRtData)
{
    ui->setupUi(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    mMPPT = 0;

    mTimer = new QTimer(this);
    mTimer->start(20);

    mSecondCounter = 0.0;
    mLastUpdateTime = 0;

    mUpdateValPlot = false;

          ui->PowerGraph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        ui->VoltageGraph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        ui->CurrentGraph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->TemperatureGraph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    //Add Power Graph traces
    int graphIndex = 0;
    ui->PowerGraph->addGraph();
    ui->PowerGraph->graph(graphIndex)->setPen(QPen(Qt::red));
    ui->PowerGraph->graph(graphIndex)->setName("Power");
    graphIndex++;

    //Add Voltage graph traces
    graphIndex = 0;
    ui->VoltageGraph->addGraph();
    ui->VoltageGraph->graph(graphIndex)->setPen(QPen(Qt::darkGreen));
    ui->VoltageGraph->graph(graphIndex)->setName("InputVoltage");
    graphIndex++;

    ui->VoltageGraph->addGraph();
    ui->VoltageGraph->graph(graphIndex)->setPen(QPen(Qt::darkBlue));
    ui->VoltageGraph->graph(graphIndex)->setName("InputVoltage");
    graphIndex++;

    //Add Current graph traces.
    graphIndex = 0;
    ui->CurrentGraph->addGraph();
    ui->CurrentGraph->graph(graphIndex)->setPen(QPen(Qt::green));
    ui->CurrentGraph->graph(graphIndex)->setName("Input current");
    graphIndex++;
    ui->CurrentGraph->addGraph();
    ui->CurrentGraph->graph(graphIndex)->setPen(QPen(Qt::blue));
    ui->CurrentGraph->graph(graphIndex)->setName("Output current");
    graphIndex++;

    //Add Temperature graph traces
    graphIndex = 0;
    ui->TemperatureGraph->addGraph();
    ui->TemperatureGraph->graph(graphIndex)->setPen(QPen(Qt::red));
    ui->TemperatureGraph->graph(graphIndex)->setName("Heatsink temeprature");
    graphIndex++;

    ui->TemperatureGraph->addGraph();
    ui->TemperatureGraph->graph(graphIndex)->setPen(QPen(Qt::green));
    ui->TemperatureGraph->graph(graphIndex)->setName("Ambient temeprature");
    graphIndex++;

    ui->TemperatureGraph->addGraph();
    ui->TemperatureGraph->graph(graphIndex)->setPen(QPen(Qt::darkYellow));
    ui->TemperatureGraph->graph(graphIndex)->setName("MCU temeprature");
    graphIndex++;

    QFont legendFont = font();
    legendFont.setPointSize(9);

    //Power
    ui->PowerGraph->legend->setVisible(true);
    ui->PowerGraph->legend->setFont(legendFont);
    ui->PowerGraph->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignBottom);
    ui->PowerGraph->legend->setBrush(QBrush(QColor(255,255,255,230)));
    ui->PowerGraph->xAxis->setLabel("Seconds (s)");
    ui->PowerGraph->yAxis->setLabel("Power (W)");
    //ui->PowerGraph->yAxis2->setLabel("Current (A)");
    ui->PowerGraph->yAxis->setRange(0, 60);
    //ui->PowerGraph->yAxis2->setRange(-5, 5);
    //ui->PowerGraph->yAxis2->setVisible(true);

    //Voltage graph
    ui->VoltageGraph->legend->setVisible(true);
    ui->VoltageGraph->legend->setFont(legendFont);
    ui->VoltageGraph->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignBottom);
    ui->VoltageGraph->legend->setBrush(QBrush(QColor(255,255,255,230)));
    ui->VoltageGraph->xAxis->setLabel("Seconds (s)");
    ui->VoltageGraph->yAxis->setLabel("Voltage (V)");
    //ui->VoltageGraph->yAxis2->setLabel("Current (A)");
    ui->VoltageGraph->yAxis->setRange(0, 60);
   // ui->VoltageGraph->yAxis2->setRange(-5, 5);
    //ui->VoltageGraph->yAxis2->setVisible(true);

    //Current Graph
    ui->CurrentGraph->legend->setVisible(true);
    ui->CurrentGraph->legend->setFont(legendFont);
    ui->CurrentGraph->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignBottom);
    ui->CurrentGraph->legend->setBrush(QBrush(QColor(255,255,255,230)));
    ui->CurrentGraph->xAxis->setLabel("Seconds (s)");
    ui->CurrentGraph->yAxis->setLabel("Current (A)");
    ui->CurrentGraph->yAxis->setRange(0, 4.2);

    //Temperature Graph
    ui->TemperatureGraph->legend->setVisible(true);
    ui->TemperatureGraph->legend->setFont(legendFont);
    ui->TemperatureGraph->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignBottom);
    ui->TemperatureGraph->legend->setBrush(QBrush(QColor(255,255,255,230)));
    ui->TemperatureGraph->xAxis->setLabel("Seconds (s)");
    ui->TemperatureGraph->yAxis->setLabel("Temperature (\u00B0C)");
    ui->TemperatureGraph->yAxis->setRange(0, 60);


    connect(mTimer, SIGNAL(timeout()),this, SLOT(timerSlot()));
}

PageRtData::~PageRtData()
{
    delete ui;
}

MPPTInterface *PageRtData::bms() const
{
    return mMPPT;
}

void PageRtData::setMPPT(MPPTInterface *dieBieMS)
{
    mMPPT = dieBieMS;

    if (mMPPT) {
        connect(mMPPT->commands(), SIGNAL(valuesReceived(MPPT_VALUES)),this, SLOT(valuesReceived(MPPT_VALUES)));
        connect(mMPPT->commands(), SIGNAL(cellsReceived(int,QVector<double>)),this, SLOT(cellsReceived(int,QVector<double>)));
    }
}

void PageRtData::timerSlot()
{
    if (mUpdateValPlot) {
        int dataSize = mSeconds.size();
        QVector<double> xAxis(dataSize);
        for (int i = 0;i < mSeconds.size();i++) {
            xAxis[i] = mSeconds[i];
        }
        // Power
        int graphIndex = 0;
        ui->PowerGraph->graph(graphIndex++)->setData(xAxis,mPower);

        graphIndex = 0;
        ui->CurrentGraph->graph(graphIndex++)->setData(xAxis,mIin);
        ui->CurrentGraph->graph(graphIndex++)->setData(xAxis,mIout);

        graphIndex = 0;
        ui->VoltageGraph->graph(graphIndex++)->setData(xAxis,mVin);
        ui->VoltageGraph->graph(graphIndex++)->setData(xAxis,mVout);

        graphIndex = 0;
        ui->TemperatureGraph->graph(graphIndex++)->setData(xAxis,mTemperatureAmbient);
        ui->TemperatureGraph->graph(graphIndex++)->setData(xAxis,mTemperatureHeatsink);
        ui->TemperatureGraph->graph(graphIndex++)->setData(xAxis,mTemperatureMCU);




        if (ui->autoscaleButton->isChecked()) {
            ui->PowerGraph->rescaleAxes();
            ui->VoltageGraph->rescaleAxes();
            ui->CurrentGraph->rescaleAxes();
            ui->TemperatureGraph->rescaleAxes();
        }

        ui->PowerGraph->replot();
        ui->VoltageGraph->replot();
        ui->CurrentGraph->replot();
        ui->TemperatureGraph->replot();


        mUpdateValPlot = false;
    }
}

void PageRtData::valuesReceived(MPPT_VALUES values)
{
    //ui->rtText->setValues(values);

    const int maxS = 500;

    appendDoubleAndTrunc(&mIin,               values.Iin,                maxS);
    appendDoubleAndTrunc(&mIout,              values.Iout,               maxS);
    appendDoubleAndTrunc(&mVin,               values.Vin,                maxS);
    appendDoubleAndTrunc(&mVout,              values.Vout,               maxS);
    appendDoubleAndTrunc(&mTemperatureAmbient,values.TemperatureAmbient, maxS);
    appendDoubleAndTrunc(&mTemperatureHeatsink,values.TemperatureHeatsink,maxS);
    appendDoubleAndTrunc(&mTemperatureMCU,     values.TemperatureMCU,    maxS);
    appendDoubleAndTrunc(&mIpv,               values.Ipv,                maxS);
    appendDoubleAndTrunc(&mPower,            values.Power,               maxS);

    //GetDate
    qint64 tNow = QDateTime::currentMSecsSinceEpoch();

    double elapsed = (double)(tNow - mLastUpdateTime) / 1000.0;
    if (elapsed > 1.0) {
        elapsed = 1.0;
    }

    mSecondCounter += elapsed;

    appendDoubleAndTrunc(&mSeconds, mSecondCounter, maxS);

    mLastUpdateTime = tNow;

    mUpdateValPlot = true;
}

void PageRtData::cellsReceived(int cellCount, QVector<double> cellVoltageArray){

}

void PageRtData::appendDoubleAndTrunc(QVector<double> *vec, double num, int maxSize)
{
    vec->append(num);

    if(vec->size() > maxSize) {
        vec->remove(0, vec->size() - maxSize);
    }
}

void PageRtData::updateZoom()
{
    Qt::Orientations plotOrientations = (Qt::Orientations)
            ((ui->zoomHButton->isChecked() ? Qt::Horizontal : 0) |
             (ui->zoomVButton->isChecked() ? Qt::Vertical : 0));

    ui->PowerGraph->axisRect()->setRangeZoom(plotOrientations);
    ui->VoltageGraph->axisRect()->setRangeZoom(plotOrientations);
    ui->CurrentGraph->axisRect()->setRangeZoom(plotOrientations);
    ui->TemperatureGraph->axisRect()->setRangeZoom(plotOrientations);
}

void PageRtData::on_zoomHButton_toggled(bool checked)
{
    (void)checked;
    updateZoom();
}

void PageRtData::on_zoomVButton_toggled(bool checked)
{
    (void)checked;
    updateZoom();
}

void PageRtData::on_rescaleButton_clicked()
{
    ui->PowerGraph->rescaleAxes();
    ui->VoltageGraph->rescaleAxes();
    ui->CurrentGraph->rescaleAxes();
    ui->TemperatureGraph->rescaleAxes();

    ui->PowerGraph->replot();
    ui->VoltageGraph->replot();
    ui->CurrentGraph->replot();
    ui->TemperatureGraph->replot();
}

void PageRtData::on_tempShowBMSBox_toggled(bool checked)
{
    //ui->tempGraph->graph(0)->setVisible(checked);
    //ui->tempGraph->graph(1)->setVisible(checked);
}

void PageRtData::on_tempShowBatteryBox_toggled(bool checked)
{
    //ui->tempGraph->graph(2)->setVisible(checked);
    //ui->tempGraph->graph(3)->setVisible(checked);
}
