#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE *fp = fopen("chart.html", "w");
    if(fp == NULL)
    {
        perror("fopen");
    
    return 1;
    }
    // meta 设为 GBK 适配Windows
    fprintf(fp,
    "<!DOCTYPE html>\n"
    "<html lang='zh-CN'>\n" //告诉浏览器这是一个中文网页 
    "<head>\n"
    "<meta charset='GBK'>\n"//保证不乱码 
    "<title>C语言ECharts图表</title>\n" //浏览器上显示（C语言ECharts表） 
    "<script src='https://cdn.bootcdn.net/ajax/libs/echarts/5.4.3/echarts.min.js'></script>\n"//引入ECharts画图库 
    "<style>#main{width:800px;height:500px;}</style>\n" // 设置图表大小 
    "</head>\n"
    "<body>\n"
    "<div id='main'></div>\n"//画图表的盒子 
    "<script>\n"
    "var myChart = echarts.init(document.getElementById('main'));\n" //创建一个图表 
    "var option = {\n"
    "title:{text:'数据统计图表'},\n" // 标题 
    "xAxis:{data:['一月','二月','三月','四月']},\n" // x轴 
    "yAxis:{},\n" // y轴 
    "series:[{name:'销量',type:'bar',data:[120,200,150,80]}]\n" // 柱状图数据 
    "};\n"
    "myChart.setOption(option);\n" // 把配置图画出来 
    "</script>\n"
    "</body>\n"
    "</html>");// 结束 

    fclose(fp);
    system("start chart.html");
    return 0;
}
