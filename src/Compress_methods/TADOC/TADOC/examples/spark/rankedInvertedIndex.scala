
package zwift

import org.apache.spark._
import org.apache.spark.SparkContext._

object mainZwift{
    def main(args: Array[String]) {
val outputFile = "hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/resRankedIISparkSeqDirectGood"
      val conf = new SparkConf().setAppName("rankedInvertedIndexSparkSeqDirectGood")
      val sc = new SparkContext(conf)


      val inputDic =  sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqGoodDic/dictionary.dic")
      val rdd0 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFileGoodDic/fusion6Input0",1)
      val rdd1 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFileGoodDic/fusion6Input1",1)
      val rdd2 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFileGoodDic/fusion6Input2",1)
      val rdd3 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFileGoodDic/fusion6Input3",1)
      val rdd4 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFileGoodDic/fusion6Input4",1)
      val rdd5 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFileGoodDic/fusion6Input5",1)


      val rdd = rdd0.repartition(1) ++ rdd1.repartition(1) ++ rdd2.repartition(1) ++ rdd3.repartition(1) ++ rdd4.repartition(1) ++ rdd5.repartition(1)
    
      val scriptPath = "/home/ubuntu/spark-2.1.0-bin-hadoop2.7/rankedInvertedIndexSparkSeqDirect/a.out"

      val pipeRDD = rdd.pipe(scriptPath).map(fstr).groupBy(_._1).saveAsTextFile(outputFile)

    }
    def fstr(s: String) : (String, (Int, Int) ) = {
      val split = s.split(" ")
        (split(0), (split(1).toInt, split(2).toInt) )
    }
  
}
