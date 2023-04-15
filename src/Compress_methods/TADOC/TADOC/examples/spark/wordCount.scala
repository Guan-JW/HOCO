
package zwift

import org.apache.spark._
import org.apache.spark.SparkContext._

object mainZwift{
    def main(args: Array[String]) {
val outputFile = "hdfs://ec2-35-162-58-53.us-west-2.compute.amazonaws.com:9000/wordcountSeq"
      val conf = new SparkConf().setAppName("wordCountSeq")
      val sc = new SparkContext(conf)

      val inputDic =  sc.textFile("hdfs://ec2-35-162-58-53.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeq/dictionary.dic")
      val rdd1 =sc.textFile("hdfs://ec2-35-162-58-53.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeq/rowCol1.dic",1)
      val rdd2 =sc.textFile("hdfs://ec2-35-162-58-53.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeq/rowCol2.dic",1)
      val rdd3 =sc.textFile("hdfs://ec2-35-162-58-53.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeq/rowCol3.dic",1)
      val rdd4 =sc.textFile("hdfs://ec2-35-162-58-53.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeq/rowCol4.dic",1)
      val rdd5 =sc.textFile("hdfs://ec2-35-162-58-53.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeq/rowCol5.dic",1)
      val rdd6 =sc.textFile("hdfs://ec2-35-162-58-53.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeq/rowCol6.dic",1)


      val rdd = rdd1 ++ rdd2 ++ rdd3 ++ rdd4 ++ rdd5 ++ rdd6
    
      val scriptPath = "/home/ubuntu/spark-2.0.0-bin-hadoop2.7/wordCountSparkSeq/a.out"

      val dictionary = inputDic.map(_.split(" ")).filter(_.size > 1).map(a => (a(0).toInt, a(1).toString))

      val pipeRDD = rdd.pipe(scriptPath).map(fstr).reduceByKey((a,b) => a+b)
	val pipeRDD2 = pipeRDD.join(dictionary)
      pipeRDD2.saveAsTextFile(outputFile) 


    }

  
    def fstr(s: String) : (Int, Int) = {
      val split = s.split(" ")
        (split(0).toInt, split(1).toInt)
    }

}
